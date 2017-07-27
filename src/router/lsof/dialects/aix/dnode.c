/*
 * dnode.c - AIX node reading functions for lsof
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
static char *rcsid = "$Id: dnode.c,v 1.25 2008/10/21 16:14:18 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local definitions
 */

#if	AIXV<5000
#define	FL_NEXT	next
#else	/* AIXV>=5000 */
#define	FL_NEXT	fl_next
# if	!defined(ISVDEV)
#define ISVDEV(t) (((t)==VBLK)||((t)==VCHR)||((t)==VFIFO)||((t)==VMPC))
# endif	/* !defined(ISVDEV) */
#endif	/* AIXV<5000 */


# if	defined(HAS_NFS)
#  if	AIXV<4210
#include <nfs/rnode.h>
#  else	/* AIXV>=4210 */
#   if	AIXA<2
/*
 * Private rnode struct definitions for AIX 4.2.1 and above
 *
 * The rnode struct IBM ships in <nfs/rnode.h> doesn't match the one
 * the kernel uses.  The kernel's rnode struct definition comes from
 * <oncplus/nfs/rnode.h>, a header file IBM does not ship with AIX.
 *
 * The rnode64 struct is for AIX above 4.3.3 whose "width" is 64.
 * (See dnode.c for the method used to determine width.)
 */

struct rnode {
	caddr_t	r_d1[11];		/* dummies; links? */
	struct vnode r_vnode;		/* vnode for remote file */
	struct gnode r_gnode;		/* gnode for remote file */
	caddr_t r_d2[29];		/* dummies; rnode elements? */
	off_t r_size;			/* client's view of file size (long)*/
	struct vattr r_attr;		/* cached vnode attributes */
};

#    if	AIXV>4330
struct rnode64 {

#     if AIXV<5200
	caddr_t	r_d1[11];		/* dummies; links? */
#     else /* AIXV>=5200 */
#      if	AIXV<5300
	caddr_t	r_d1[12];		/* dummies; links? */
#      else	/* AIXV>=5300 */
	caddr_t r_d1[7];		/* dummies; links? */
#      endif	/* AIXV<5300 */
#     endif /* AIXV<5200 */

	struct vnode r_vnode;		/* vnode for remote file */
	struct gnode r_gnode;		/* gnode for remote file */

#     if	AIXV<5300
	caddr_t r_d2[15];		/* dummies; rnode elements? */
#     else	/* AIXV>=5300 */
	caddr_t r_d2[11];		/* dummies; rnode elements? */
#     endif	/* AIXV<5300 */

	off_t r_size;			/* client's view of file size (long)*/
	struct vattr r_attr;		/* cached vnode attributes */
};
#    endif	/* AIXV>4330 */
#   else	/* AIXA>=2 */
struct rnode {
	KA_T d1[7];			/* dummies */
	struct vnode r_vnode;		/* vnode for remote file */
	struct gnode r_gnode;		/* gnode for remote file */
	KA_T d2[19];			/* dummies */
	off_t r_size;			/* client's view of file size (long)*/
	struct vattr r_attr;		/* cached vnode attributes */
};
#   endif	/* AIXA<2 */
#  endif	/* AIXV<4210 */
# endif	/* defined(HAS_NFS) */


/*
 * isglocked() - is a gnode locked
 */

char
isglocked(ga)
	struct gnode *ga;		/* local gnode address */
{

	struct filock *cfp, f, *ffp;
	int l;

	if (!(ffp = ga->gn_filocks))
	    return(' ');
	cfp = ffp;

#if	AIXV>=4140
	do {
#endif	/* AIXV>=4140 */

	    if (kread((KA_T)cfp, (char *)&f, sizeof(f)))
		return(' ');

#if	AIXV>=4140
	    if (f.set.l_sysid || f.set.l_pid != (pid_t)Lp->pid)
		continue;
#endif	/* AIXV>=4140 */

	    if (f.set.l_whence == 0 && f.set.l_start == 0

#if	AIXV>=4200
	    &&  f.set.l_end == 0x7fffffffffffffffLL
#else	/* AIXV<4200 */
	    &&  f.set.l_end == 0x7fffffff
#endif	/* AIXV>=4200 */

	    )
		l = 1;
	    else
		l = 0;
	    switch (f.set.l_type & (F_RDLCK | F_WRLCK)) {

	    case F_RDLCK:
		return((l) ? 'R' : 'r');
	    case F_WRLCK:
		return((l) ? 'W' : 'w');
	    case (F_RDLCK + F_WRLCK):
		return('u');
	    }
	    return(' ');

#if	AIXV>=4140
	} while ((cfp = f.FL_NEXT) && cfp != ffp);
	return(' ');
#endif	/* AIXV>=4140 */

}


/*
 * process_node() - process vnode
 */

void
process_node(va)
	KA_T va;			/* vnode kernel space address */
{
	struct cdrnode c;
	dev_t dev, rdev;
	int devs = 0;
	struct gnode g;
	struct l_ino i;
	int ic = 0;
	int ins = 0;
	struct vfs *la = NULL;
	int rdevs = 0;
	size_t sz;
	char tbuf[32], *ty;
	enum vtype type;
	struct l_vfs *vfs;
	static struct vnode *v = (struct vnode *)NULL;

#if	AIXV>=3200
	struct devnode dn;
	struct gnode pg;
	struct specnode sn;
	struct fifonode f;
#endif	/* AIXV>=3200 */

#if	defined(HAS_AFS)
	static int afs = 0;		/* AFS test status: -1 = no AFS
					 *		     0 = not tested
					 *		     1 = AFS present */
	struct afsnode an;
#endif	/* defined(HAS_AFS) */

#if	defined(HAS_NFS)
	struct vattr nfs_attr;
	int nfss = 0;
	static struct rnode r;
	static char *rp = (char *)&r;
	static int rsz = sizeof(r);

# if	AIXV>4330 && AIXA<2
	static struct rnode64 r64;
# endif	/* AIXV>4330 && AIXA<2 */

# if	AIXA<2
	static int width = -1;
# else	/* AIXA>=2 */
	static width = 64;
# endif	/* AIXA<2 */
#endif	/* defined(HAS_NFS) */

#if	defined(HAS_SANFS)
	struct sanfs_node {	/* DEBUG */

	/*
	 * This is a DEBUG version of the SANFS node structure.  When IBM makes
	 * the SANFS header files available in /usr/include, this definition
	 * will be removed.
	 */
	    u_long san_d1[20];	/* DEBUG */
	    struct gnode san_gnode;	/* DEBUG */
	    u_long san_d2[128];	/* DEBUG */
	} san;
	int sans = 0;
#endif	/* defined(HAS_SANFS) */

#if	AIXV>=4140
	struct clone *cl;
	KA_T ka;
	struct module_info mi;
	int ml, nx;
	char mn[32];
	struct queue q;
	struct qinit qi;
	KA_T qp, xp;
	int ql;
	struct sth_s {			/* stream head */
	    KA_T *dummy;		/* dummy */
	    KA_T *sth_wq;		/* write queue */
	} sh;
	struct xticb {			/* XTI control block */
	    int d1;
	    long d2;
	    int d3;
	    struct socket *xti_so;	/* socket pointer */
	} xt;
#endif	/* AIXV>=4140 */


/*
 * Read the vnode.
 */
	if (!va) {
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
	    v = (struct vnode *)malloc((MALLOC_S)sizeof(struct vnode));
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
/*
 * Read the vnode.
 */
	if (readvnode(va, v)) {
	    enter_nm(Namech);
	    return;
	}

#if	defined(HASFSTRUCT)
	Lf->fsv |= FSV_NI;
	Lf->fna = va;
#endif	/* defined(HASFSTRUCT) */

/*
 * Read the gnode.
 */
	if (!v->v_gnode || readgnode((KA_T)v->v_gnode, &g)) {
	    if (Selinet) {
		Lf->sf = SELEXCLF;
		return;
	    }
	    (void) snpf(Namech, Namechl, "vnode at %s has no gnode\n",
		print_kptr(va, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}

#if	AIXV>=3200

/*
 * Under AIX 3.2 and above, if the vnode type is ISVDEV, then there is a
 * special node and a fifonode or devnode.  Behind them are the "real"
 * gnode, inode and vnode.
 */
	if (ISVDEV(g.gn_type)) {
	    switch (g.gn_type) {
	    case VBLK:
		Ntype = N_BLK;
		break;
	    case VCHR:
		Ntype = N_CHR;
		break;
	    case VFIFO:
		Ntype = N_FIFO;
		break;
	    case VMPC:
		Ntype = N_MPC;
		break;
	    default:
		(void) snpf(Namech, Namechl, "vnode at %s: unknown ISVDEV(%#x)",
		    print_kptr(va, (char *)NULL, 0), g.gn_type);
		enter_nm(Namech);
		return;
	    }
	/*
	 * Read the special node.
	 */
	    if (!g.gn_data || kread((KA_T)g.gn_data, (char *)&sn, sizeof(sn))) {
		if (Selinet) {
		    Lf->sf = SELEXCLF;
		    return;
		}
		(void) snpf(Namech, Namechl,
		    "vnode at %s: can't read specnode (%s)",
		    print_kptr(va, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)g.gn_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	   }
	/*
	 * Read the PFS gnode and its inode and vnode.
	 */
	    if (sn.sn_pfsgnode) {
		if (Selinet) {
		    Lf->sf = SELEXCLF;
		    return;
		}
		if (readgnode((KA_T)sn.sn_pfsgnode, &g)) {
		    (void) snpf(Namech, Namechl,
			"vnode at %s: can't read pfsgnode (%s)",
			print_kptr(va, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)sn.sn_pfsgnode, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		if (!g.gn_data || readlino(&g, &i)) {
		    (void) snpf(Namech, Namechl,
			"pfsgnode at %s: can't read inode (%s)",
			print_kptr((KA_T)sn.sn_pfsgnode, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)g.gn_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		ins = 1;
		if (!g.gn_vnode || readvnode((KA_T)g.gn_vnode, v)) {
		    (void) snpf(Namech, Namechl,
			"pfsgnode at %s: can't read vnode (%s)",
			print_kptr((KA_T)sn.sn_pfsgnode, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)g.gn_vnode, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
	    } else {
		(void) zeromem((char *)&i, sizeof(i));

#if	AIXV>=4140
	    /*
	     * See if this is a clone device, connected to a stream.
	     *
	     *     the clone major device number must be known;
	     *     the specnode must have a devnode pointer;
	     *     and the devnode must have a stream head pointer.
	     */
		if (CloneMaj >= 0
		&&  sn.sn_devnode
		&&  kread((KA_T)sn.sn_devnode, (char *)&dn, sizeof(dn)) == 0
		&&  (ka = (KA_T)dn.dv_pdata))
		{

# if	defined(HASDCACHE)

process_clone_again:

# endif	/* defined(HASDCACHE) */

		    for (cl = Clone; cl; cl = cl->next) {
			if (GET_MAJ_DEV(g.gn_rdev) == GET_MIN_DEV(cl->cd.rdev))
			{

# if	defined(HASDCACHE)
			    if (DCunsafe && !cl->cd.v && !vfy_dev(&cl->cd))
				goto process_clone_again;
# endif	/* defined(HASDCACHE) */

			/*
			 * Identify this file as a clone.  Save the clone
			 * device inode number as the file's inode number.
			 */
			    ic = 1;
			    Lf->inode = cl->cd.inode;
			    Lf->inp_ty = 1;
			    if (ClonePtc >= 0
			    &&  GET_MAJ_DEV(g.gn_rdev) == ClonePtc) {
				if (Selinet) {
				    Lf->sf = SELEXCLF;
				    return;
				}
			    /*
			     * If this is a /dev/ptc stream, enter the device
			     * name and the channel.
			     */
			        (void) snpf(Namech, Namechl, "%s/%d",
				    cl->cd.name, (int)GET_MIN_DEV(g.gn_rdev));
				break;
			    }
			/*
			 * If this isn't a /dev/ptc stream, collect the names
			 * of the modules on the stream.  Ignore the stream
			 * head and look for an "xtiso" module.  Limit the
			 * module depth to 25.
			 */
			    (void) snpf(Namech, Namechl, "STR:%s", cl->cd.name);
			    nx = (int) strlen(Namech);
			    if (!kread(ka, (char *)&sh, sizeof(sh)))
				qp = (KA_T)sh.sth_wq;
			    else
				qp = (KA_T)NULL;
			    for (mn[sizeof(mn) - 1] = '\0', ql = 0;
				 qp && (ql < 25);
				 ql++, qp = (KA_T)q.q_next)
			    {

			    /*
			     * Read the queue structure.  If it can't be read,
			     * end module name collection.
			     *
			     * The queue structure should lead to a qinfo
			     * structure, and the qinfo structure should lead
			     * to a module_info structure, where the module
			     * name should be found.  If there's no queue
			     * structure.
			     *
			     * If the qinfo or module_info structures can't be
			     * read, skip to the next queue structure.
			     */
				if (kread(qp, (char *)&q, sizeof(q)))
				    break;
				if (!(ka = (KA_T)q.q_qinfo)
				||  kread(ka, (char *)&qi, sizeof(qi)))
				    continue;
				if (!(ka = (KA_T)qi.qi_minfo)
				||  kread(ka, (char *)&mi, sizeof(mi)))
				    continue;
				if (!(ka = (KA_T)mi.mi_idname)
				||  kread(ka, mn, sizeof(mn) - 1)
				||  !(ml = (int) strlen(mn))
				||  !strcmp(mn, "sth"))
				    continue;
				if (!strcmp(mn, "xtiso")
				&&  (xp = (KA_T)q.q_ptr)
				&&  !kread(xp, (char *)&xt, sizeof(xt))
				&&  (ka = (KA_T)xt.xti_so)) {

				/*
				 * The xtiso module's private queue pointer
				 * leads to an xticb with a non-NULL socket
				 * pointer.  Process the stream as a socket.
				 */
				    Namech[0] = '\0';
				    Lf->inp_ty = 0;
				    (void) process_socket(ka);
				    return;
				}
			    /*
			     * Save the module name in Mamech[] as a "->"
			     * prefixed chain, beginning with "STR:<device>".
			     */
				if ((nx + ml + 2) > (Namechl - 1))
				    continue;
				(void) snpf(&Namech[nx], Namechl, "->%s", mn);
				nx += (ml + 2);
			    }
			    break;
			}
		    }
		}
#endif	/* AIXV>=4140 */

		if (Selinet) {
		    Lf->sf = SELEXCLF;
		    return;
		}
	    }
	/*
	 * If it's a FIFO, read its fifonode.
	 */
	    if (Ntype == N_FIFO) {
		if (!sn.sn_fifonode ||readfifonode((KA_T)sn.sn_fifonode, &f)) {
		    (void) snpf(Namech, Namechl,
			"vnode at %s: can't read fifonode (%s)",
			print_kptr(va, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)sn.sn_fifonode, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
	/*
	 * Otherwise, read the devnode and its gnode.
	 */
	    } else {
		if (!sn.sn_devnode
		|| kread((KA_T)sn.sn_devnode,(char *)&dn,sizeof(dn))) {
		    (void) snpf(Namech, Namechl,
			"vnode at %s: can't read devnode (%s)",
			print_kptr(va, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)sn.sn_devnode, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		g = dn.dv_gnode;
	    }
	}
#endif	/* AIXV>=3200 */

/*
 * Read the AIX virtual file system structure.
 */
	if (Ntype != N_AFS && g.gn_rdev == NODEVICE) {
	    vfs = (struct l_vfs *)NULL;
	    enter_dev_ch(print_kptr(va, (char *)NULL, 0));
	} else {
	    if (!(vfs = readvfs(v))) {
		(void) snpf(Namech, Namechl, "can't read vfs for %s at %s",
		    print_kptr(va, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)v->v_vfsp, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	}
/*
 * Set special node types: NFS, PROC or SANFS.
 */

#if	defined(MNT_REMOTE)
	if (vfs && vfs->vmt_flags & MNT_REMOTE) {
	    switch(vfs->vmt_gfstype) {

# if	defined(HAS_NFS)
	    case MNT_NFS:

#  if	defined(MNT_NFS3)
	    case MNT_NFS3:
#  endif	/* defined(MNT_NFS3) */

#  if	defined(MNT_NFS4)
	    case MNT_NFS4:
#  endif	/* defined(MNT_NFS4) */

#  if	defined(HAS_AFS)
		if (!AFSVfsp || (KA_T)v->v_vfsp != AFSVfsp)
#  endif	/* defined(HAS_AFS) && defined(HAS_NFS) */

		    Ntype = N_NFS;
# endif	/* defined(HAS_NFS) */
		break;

# if	defined(HAS_SANFS) && defined(MNT_SANFS)
	    case MNT_SANFS:
		Ntype = N_SANFS;
		break;
# endif	/* defined(HAS_SANFS) && defined(MNT_SANFS) */

	    }
	}
#endif	/* defined(MNT_REMOTE) */

#if	defined(HASPROCFS)
	if (vfs && (vfs->vmt_gfstype == MNT_PROCFS))
	    Ntype = N_PROC;
#endif	/* defined(HASPROCFS) */

/*
 * Get the lock status.
 */
	Lf->lock = isglocked(&g);
	switch (Ntype) {

#if	defined(HAS_NFS)
/*
 * Read an NFS rnode.
 */
	case N_NFS:

# if	AIXA<2
	    if (width == -1) {

	    /*
	     * Establish the architecture's bit width and set NFS rnode
	     * access parameters accordingly.
	     */

#  if	AIXV<=4330
		width = 32;
#  else	/* AIXV>4330 */
		if (__KERNEL_64()) {
		    width = 64;
		    rp = (char *)&r64;
		    rsz = sizeof(r64);
		} else if (__KERNEL_32()) {
		    width = 32;
		} else {
		    if (!Fwarn)
			(void) fprintf(stderr,
			    "%s: WARNING: unknown kernel bit size\n", Pn);
		    width = -2;
		}
#  endif	/* AIXV<-4330 */

	    }
# endif	/* AIXA<2 */

	    if (width > 0) {
		if (!g.gn_data || kread((KA_T)g.gn_data, rp, rsz)) {
		    (void) snpf(Namech, Namechl,
			"remote gnode at %s has no rnode",
			print_kptr((KA_T)v->v_gnode, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}

# if	AIXV<=4330 || AIXA>=2
		nfs_attr = r.r_attr;
		nfss = 1;
# else	/* AIXV>4330 && AIXA<2 */
		switch (width) {
		case 32:
		    nfs_attr = r.r_attr;
		    nfss = 1;
		    break;
		case 64:
		    nfs_attr = r64.r_attr;
		    nfss = 1;
		    break;
		}
# endif	/* AIXV<=4330 || AIXA>=2 */

	    }
	    break;
#endif	/* defined(HAS_NFS) */

#if	defined(HAS_SANFS)
/*
 * Read SANFS node and associated structures.
 */
	case N_SANFS:
	    if (!g.gn_data
	    ||  kread((KA_T)g.gn_data, &san, sizeof(san))
	    ) {
		(void) snpf(Namech, Namechl, "gnode at %s has no SANFS node",
		    print_kptr((KA_T)v->v_gnode, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	/*
	 * DEBUG: this code is insufficient.  It can't be completed until IBM
	 * makes the SANFS header files available in /usr/include.  There are
	 * apparently two node structures following the SANFS node and file
	 * attributes (size, etc.) are in the second structure.
	 */
	    sans = 1;
	    break;
#endif	/* defined(HAS_SANFS) */

/*
 * Read N_REGLR nodes.
 */
	case N_REGLR:
	    if (vfs && vfs->vmt_gfstype == MNT_CDROM) {

	    /*
	     * Read a CD-ROM cdrnode.
	     */
		if (!g.gn_data || readcdrnode((KA_T)g.gn_data, &c)) {
		    (void) snpf(Namech, Namechl, "gnode at %s has no cdrnode",
			print_kptr((KA_T)v->v_gnode, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		(void) zeromem((char *)&i, sizeof(i));
		i.number = (INODETYPE)c.cn_inumber;
		i.size = (off_t)c.cn_size;
		i.number_def = i.size_def = 1;
	    /*
	     * Otherwise, read the inode.
	     */

	    } else if (g.gn_data) {
		if (readlino(&g, &i)) {
		    (void) snpf(Namech, Namechl,
			"gnode at %s can't read inode: %s",
			print_kptr((KA_T)v->v_gnode, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)g.gn_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		ins = 1;
	    }

#if	defined(HAS_AFS)
	    else {

	    /*
	     * See if this is an AFS node.
	     */
		if (AFSVfsp && (KA_T)v->v_vfsp == AFSVfsp)
		    Ntype = N_AFS;
		else if (v->v_vfsp) {
		    switch (afs) {
		    case -1:
			break;
		    case 0:
			if (!hasAFS(v)) {
			    afs = 1;
			    break;
			}
			afs = 1;
			Ntype = N_AFS;
			break;
		    case 1:
			if ((KA_T)v->v_vfsp == AFSVfsp)
			    Ntype = N_AFS;
		     }
		}
	    /*
	     * If this is an AFS node, read the afsnode.
	     */
		if (Ntype == N_AFS) {
		    if (readafsnode(va, v, &an))
			return;
		} else {
		    (void) snpf(Namech, Namechl, "gnode at %s has no inode",
			print_kptr((KA_T)v->v_gnode, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
	    }
#else	/* !defined(HAS_AFS) */

	    else {
		(void) snpf(Namech, Namechl, "gnode at %s has no inode",
		    print_kptr((KA_T)v->v_gnode, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
#endif	/* defined(HAS_AFS) */

	}
/*
 * Get device and type for printing.
 */

#if	defined(HAS_NFS)
	if (Ntype == N_NFS) {
	    if (vfs) {
		dev = vfs->dev;
		devs = 1;
	    }
	} else
#endif	/* defined(HAS_NFS) */

#if	defined(HAS_AFS)
	if (Ntype == N_AFS) {
	    dev = an.dev;
	    devs = 1;
	} else
#endif	/* defined(HAS_AFS) */

#if	defined(HASPROCFS)
	if (Ntype == N_PROC) {

/* WARNING!!!   WARNING!!!   The following hack should be removed ASAP!!! */
	    dev = vfs ? (vfs->dev & 0x7fffffffffffffff) : 0;
/* WARNING!!!   WARNING!!!   The above hack should be removed ASAP!!! */

	    devs = 1;
	}
	else
#endif	/* defined(HASPROCFS) */

#if	defined(HAS_SANFS)
	if ((Ntype == N_SANFS) && vfs) {
	    dev = vfs->dev;
	    devs = 1;
	}
	else
#endif	/* defined(HAS_SANFS) */

	{
	    if (vfs) {
		dev = vfs->dev;
		devs = 1;
	    }
	    rdev = g.gn_rdev;
	    rdevs = 1;
	}

#if	AIXV>=3200
	if (Ntype == N_MPC)
	    type = VMPC;
	else
#endif	/* AIXV>=3200 */

	    type = g.gn_type;
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

#if	defined(HAS_NFS)
	case N_NFS:
	    if (nfss) {
		Lf->inode = (INODETYPE)nfs_attr.va_serialno;
		Lf->inp_ty = 1;
	    }
	    break;
#endif	/* defined(HAS_NFS) */

#if	defined(HAS_SANFS)
	case N_SANFS:
	    if (sans) {
	    
	    /*
	     * DEBUG: this code is insufficient.  It can't be completed until
	     * IBM makes the SANFS header files available in /usr/include.
	     */
		/* Lf->inode = ???	DEBUG */
		Lf->inp_ty = 1;
	    }
	    break;
#endif	/* defined(HAS_SANFS) */

# if	AIXV>=3200
	case N_BLK:
	case N_CHR:
	case N_FIFO:
	case N_MPC:
# endif	/* AIXV>=3200 */

	case N_REGLR:
	    if (ins) {
		Lf->inode = (INODETYPE)i.number;
		Lf->inp_ty = i.number_def;
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

#if	AIXV>=3200
	    case N_FIFO:
		Lf->sz = (SZOFFTYPE)f.ff_size;
		Lf->sz_def = 1;
		break;
#endif	/* AIXV>=3200 */

#if	defined(HAS_NFS)
	    case N_NFS:
		if (nfss) {
		    Lf->sz = (SZOFFTYPE)nfs_attr.va_size;
		    Lf->sz_def = 1;
		}
		break;
#endif	/* defined(HAS_NFS) */

#if	defined(HAS_SANFS)
	    case N_SANFS:
		if (sans) {

		/*
	 	 * DEBUG: this code is insufficient.  It can't be completed
		 * until IBM makes the SANFS header files available in
		 * /usr/include.
		 */
		    /* Lf->sz = (SZOFFTYPE)???	DEBUG */
		    Lf->sz_def = 1;
		}
		break;
#endif	/* defined(HAS_SANFS) */

#if	 AIXV>=3200
	    case N_BLK:
		if (!Fsize)
		    Lf->off_def = 1;
		break;
	    case N_CHR:
	    case N_MPC:
		if (!Fsize)
		    Lf->off_def = 1;
		break;
#endif	/* AIXV>=3200 */

	    case N_REGLR:
		if (type == VREG || type == VDIR) {
		    if (ins) {
			Lf->sz = (SZOFFTYPE)i.size;
			Lf->sz_def = i.size_def;
		    }
		} else if (((type == VBLK) || (type == VCHR) || (type == VMPC))
		       &&  !Fsize)
		    Lf->off_def = 1;
		break;
	    }
	}
/*
 * Record link count.
 */
	if (Fnlink) {
	    switch(Ntype) {

#if	defined(HAS_AFS)
	    case N_AFS:
		Lf->nlink = an.nlink;
		Lf->nlink_def = an.nlink_st;
		break;
#endif	/* defined(HAS_AFS) */

#if	defined(HAS_NFS)
	    case N_NFS:
		if (nfss) {
		    Lf->nlink = (long)nfs_attr.va_nlink;
		    Lf->nlink_def = 1;
		}
		break;
#endif	/* defined(HAS_NFS) */

#if	defined(HAS_SANFS)
	    case N_SANFS:
		if (sans) {

		/*
	 	 * DEBUG: this code is insufficient.  It can't be completed
		 * until IBM makes the SANFS header files available in
		 * /usr/include.
		 */
		    /* Lf->nlink = (long)???	DEBUG */
		    Lf->nlink_def = 1;
		}
		break;
#endif	/* defined(HAS_SANFS) */

#if	AIXV>=3200
	    case N_BLK:
	    case N_CHR:
	    case N_FIFO:
	    case N_MPC:
#endif	/* AIXV>=3200 */

	    case N_REGLR:
		if (ins) {
		    Lf->nlink = (long)i.nlink;
		    Lf->nlink_def = i.nlink_def;
		}
		break;
	    }
	    if (Nlink && Lf->nlink_def && (Lf->nlink < Nlink))
		Lf->sf |= SELNLINK;
	}

#if	defined(HAS_NFS)
/*
 * Record an NFS file selection.
 */
	if (Ntype == N_NFS && Fnfs)
	    Lf->sf |= SELNFS;
#endif	/* defined(HAS_NFS) */

/*
 * Save the file system names.
 */
	if (vfs) {
	    Lf->fsdir = vfs->dir;
	    Lf->fsdev = vfs->fsname;
	}
/*
 * Save the device numbers and their states.
 *
 * Format the vnode type.
 */
	switch (type) {

	case VNON:
	    ty ="VNON";
	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
	    break;
	case VREG:
	case VDIR:
	    ty = (type == VREG) ? "VREG" : "VDIR";
	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
	    break;
	case VBLK:
	    ty = "VBLK";
	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
	    Ntype = N_BLK;
	    break;
	case VCHR:
	    ty = "VCHR";
	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
	    Ntype = N_CHR;
	    break;
	case VLNK:
	    ty = "VLNK";
	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
	    break;

#if	defined(VSOCK)
	case VSOCK:
	    ty = "SOCK";
	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
	    break;
#endif

	case VBAD:
	    ty = "VBAD";
	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
	    break;
	case VFIFO:
	    if (!Lf->dev_ch || Lf->dev_ch[0] == '\0') {
		Lf->dev = dev;
		Lf->dev_def = devs;
		Lf->rdev = rdev;
		Lf->rdev_def = rdevs;
	    }
	    ty = "FIFO";
	    break;
	case VMPC:
	    Lf->rdev = g.gn_rdev;
	    Lf->rdev_def = 1;
	    if (vfs) {
		Lf->dev = vfs->dev;
		Lf->dev_def = 1;
	    }
	    Lf->ch = g.gn_chan;

#if	AIXV<3200
	    Lf->inp_ty = 0;
#endif	/* AIXV<3200 */

	    Ntype = N_CHR;
	    ty = "VMPC";
	    break;
	default:
	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
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
	if (Sfile && is_file_named(NULL, type, g.gn_chan, ic))
	    Lf->sf |= SELNM;
/*
 * Enter name characters.
 */
	if (Namech[0])
	    enter_nm(Namech);
}


#if	defined(HASPRIVFILETYPE)
/*
 * process_shmt() -- process shared memory transport file
 */

void
process_shmt(sa)
	KA_T sa;			/* shared memory transport node struct
					 * address ??? */
{
	struct shmtnode {		/* shared memory transport node
					 * struct ??? */

	    struct shmtnode *peer;	/* peer shmtnode struct */
	    caddr_t d1[2];		/* dummy to fill space */
	    int sz;			/* buffer size */
	    caddr_t d2[3];		/* dyummy to fill space */
	    int free;			/* free bytes in buffer */
	    caddr_t d3[17];		/* dummy to fill space */
	    pid_t pid;			/* process ID */
	} mn, pn;
/*
 * Ignore this file if only Internet files are selected.
 */
	if (Selinet) {
	    Lf->sf |= SELEXCLF;
	    return;
	}
/*
 * Set type to " SMT" and put shmtnode structure address in device column.
 */
	(void) snpf(Lf->type, sizeof(Lf->type), " SMT");
	if (!sa || kread((KA_T)sa, (char *)&mn, sizeof(mn))) {
	    (void) snpf(Namech, Namechl, "can't read shmtnode: %s",
		print_kptr(sa, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
	enter_dev_ch(print_kptr(sa, (char *)NULL, 0));
/*
 * If offset display has been requested or if buffer size less free bytes is
 * negative, enable offset display.  Otherwise set the  file size as buffer
 * size less free bytes.
 */
	if (Foffset || mn.free > mn.sz)
	    Lf->off_def = 1;
	else {
	    Lf->sz = (SZOFFTYPE)(mn.sz - mn.free);
	    Lf->sz_def = 1;
	}
/*
 * If there is a peer, read its shmtnode structure.
 */
	if (!mn.peer)
	    (void) snpf(Namech, Namechl, "->(unknown)");
	else {
	    if (kread((KA_T)mn.peer, (char *)&pn, sizeof(pn)))
		(void) snpf(Namech, Namechl, "can't read peer shmtnode: %s",
		    print_kptr((KA_T)mn.peer, (char *)NULL, 0));
	    else {
		if (pn.pid)
		    (void) snpf(Namech, Namechl, "->%s (PID %d)",
			print_kptr((KA_T)mn.peer, (char *)NULL, 0), pn.pid);
		else
		    (void) snpf(Namech, Namechl, "->%s",
			print_kptr((KA_T)mn.peer, (char *)NULL, 0));
	    }
	}
	enter_nm(Namech);
}
#endif	/* AIXV>=4200 */


/*
 * readlino() -- read local inode
 */

int
readlino(ga, li)
	struct gnode *ga;			/* gnode address */
	struct l_ino *li;			/* local inode receiver */
{
	struct inode i;				/* "regular" inode */

#if	defined(HAS_JFS2)
	static struct vnodeops *j2va = (struct vnodeops *)NULL;
						/* j2_vnops address */
	static int j2vas = 0;			/* j2nl[] status */
#endif	/* defined(HAS_JFS2) */

	zeromem((char *)li, sizeof(struct l_ino));
	if (!ga || !ga->gn_data)
	    return(0);

#if	defined(HAS_JFS2)
	if (!j2vas) {

	/*
	 * Get the j2_vnops address once.
	 */
	    struct nlist j2nl[] = {
		{ "j2_vnops"	},
		{ (char *)NULL	}
	    };

	    if (nlist(N_UNIX, j2nl) == 0)
		j2va = (struct vnodeops *)j2nl[0].n_value;
	    if (!j2va && !Fwarn) {
		(void) fprintf(stderr,
		    "%s: WARNING: can't identify jfs2 files\n", Pn);
	    }
	    j2vas = 1;
	}
/*
 * If this system has jfs2, see if this gnode's operation structure pointer
 * references j2_vnops.
 */
	if (ga->gn_ops && j2va && (ga->gn_ops == j2va))
	    return(readj2lino(ga, li));
#endif	/* defined(HAS_JFS2) */

/*
 * Read a "standard" inode.
 */
	if (readinode((KA_T)ga->gn_data, &i))
	    return(1);
	li->dev = i.i_dev;
	li->nlink = i.i_nlink;
	li->number = (INODETYPE)i.i_number;
	li->size = i.i_size;
	li->dev_def = li->nlink_def = li->number_def = li->size_def = 1;
	return(0);
}
