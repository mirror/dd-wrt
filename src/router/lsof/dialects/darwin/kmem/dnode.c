/*
 * dnode.c - Darwin node functions for /dev/kmem-based lsof
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
static char *rcsid = "$Id: dnode.c,v 1.11 2006/03/27 23:24:50 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local function prototypes
 */

#if	DARWINV<600
_PROTOTYPE(static int lkup_dev_tty,(dev_t *dr, dev_t *rdr, INODETYPE *ir));
#endif	/* DARWINV<600 */

#if	DARWINV>=800
_PROTOTYPE(static char *getvpath,(KA_T va, struct vnode *rv));
_PROTOTYPE(static int readvname,(KA_T addr, char *buf, int buflen));
#endif	/* DARWINV>=800 */


#if	DARWINV>=800
/*
 * getvpath() - get vnode path
 *		adapted from build_path() (.../bsd/vfs/vfs_subr.c)
 */

static char *
getvpath(va, rv)
	KA_T va;			/* kernel address of the rightmost
					 * vnode in the path */
	struct vnode *rv;		/* pointer to rightmost vnode */
{
	char *ap;
	static char *bp = (char *)NULL;
	static size_t bl = (size_t)(MAXPATHLEN + MAXPATHLEN + 1);
	static char *cb = (char *)NULL;
	static size_t cbl = (size_t)0;
	static int ce = 0;
	struct mount mb;
	int pl, vnl;
	char *pp, vn[MAXPATHLEN+1];
	struct vnode vb;
	KA_T vas = va;
/*
 * Initialize the path assembly.
 */
	if (!bp) {
	    if (!(bp = (char *)malloc((MALLOC_S)bl))) {
		(void) fprintf(stderr, "%s: no space (%d) for path assembly\n",
		    Pn, (int)bl);
		Exit(1);
	    }
	}
	pp = bp + bl - 1;
	*pp = '\0';
	pl = 0;
/*
 * Process the starting vnode.
 */
	if (!va)
	    return(0);
	if ((rv->v_flag & VROOT) && rv->v_mount) {

	/*
	 * This is the root of a file system and it has a mount structure.
	 * Read the mount structure.
	 */
	    if (kread((KA_T)rv->v_mount, (char *)&mb, sizeof(mb)))
		return(0);
	    if (mb.mnt_flag & MNT_ROOTFS) {

	    /*
	     * This is the root file system, so the path is "/".
	     */
		pp--;
		*pp = '/';
		pl = 1;
		goto getvpath_alloc;
	    } else {

	    /*
	     * Get the covered vnode's pointer and read it.  Use it to
	     * form the path.
	     */
		if ((va = (KA_T)mb.mnt_vnodecovered)) {
		    if (readvnode(va, &vb))
			return(0);
		}
	    }
	} else {

	/*
	 * Use the supplied vnode.
	 */
	    vb = *rv;
	}
/*
 * Accumulate the path from the vnode chain.
 */
	while (va && ((KA_T)vb.v_parent != va)) {
	    if (!vb.v_name) {

	    /*
	     * If there is no name pointer or parent, the assembly is complete.
	     */
		if (vb.v_parent) {

		/*
		 * It is an error if there is a parent but no name.
		 */
		    return((char *)NULL);
		}
		break;
	    }
	/*
	 * Read the name and add it to the assembly.
	 */
	    if ((vnl = readvname((KA_T)vb.v_name, vn, sizeof(vn))) <= 0)
		return((char *)NULL);
	    if ((vnl + 1 + pl + 1) > bl)
		return((char *)NULL);
	    memmove((void *)(pp - vnl), (void *)vn, vnl);
	    pp -= (vnl + 1);
	    *pp = '/';
	    pl += vnl + 1;
	    if ((va == vas) && (vb.v_flag & VROOT)) {

	    /*
	     * This is the starting vnode and it is a root vnode.  Read its
	     * mount structure.
	     */
		if (vb.v_mount) {
		    if (kread((KA_T)vb.v_mount, (char *)&mb, sizeof(mb)))
			return((char *)NULL);
		    if (mb.mnt_vnodecovered) {

		    /*
		     * If there's a covered vnode, read it and use it's parent
		     * vnode pointer.
		     */
			if ((va = (KA_T)mb.mnt_vnodecovered)) {
			    if (readvnode(va, &vb))
				return((char *)NULL);
			    va = (KA_T)vb.v_parent;
			}
		    } else
			va = (KA_T)NULL;
		} else
		    va = (KA_T)NULL;
	    } else
		va = (KA_T)vb.v_parent;
	/*
	 * If there's a parent vnode, read it.
	 */
	    if (va) {
		if (readvnode(va, &vb))
		    return((char *)NULL);
		if ((vb.v_flag & VROOT) && vb.v_mount) {

		/*
		 * The mount point has been reached.  Read the mount structure
		 * and use its covered vnode pointer.
		 */
		    if (kread((KA_T)vb.v_mount, (char *)&mb, sizeof(mb)))
			return((char *)NULL);
		    if ((va = (KA_T)mb.mnt_vnodecovered)) {
			if (readvnode(va, &vb))
			    return((char *)NULL);
		    }
		}
	    }
	}
/*
 * As a special case the following code attempts to trim a path that is
 * larger than MAXPATHLEN by seeing if the lsof process CWD can be removed
 * from the start of the path to make it MAXPATHLEN characters or less.
 */
	if (pl > MAXPATHLEN) {

	/*
	 * Get the cwd.  If that can't be done, return an error.
	 */
	    if (ce)
		return((char *)NULL);
	    if (!cb) {
		if (!(cb = (char *)malloc((MALLOC_S)(MAXPATHLEN + 1)))) {
		    (void) fprintf(stderr, "%s: no space (%d) for CWD\n",
			Pn, (int)bl);
		    Exit(1);
		}
		if (!getcwd(cb, (size_t)(MAXPATHLEN + 1))) {
		    if (!Fwarn) {
			(void) fprintf(stderr, "%s: WARNING: can't get CWD\n",
			    Pn);
		    }
		    ce = 1;
		    return((char *)NULL);
		}
		cb[MAXPATHLEN - 1] = '\0';
		if (!(cbl = (size_t)strlen(cb))) {
		    if (!Fwarn) {
			(void) fprintf(stderr, "%s: WARNING: CWD is NULL\n",
			    Pn);
		    }
		    ce = 1;
		    return((char *)NULL);
		}
	    }
	/*
	 * See if trimming the CWD shortens the path to MAXPATHLEN or less.
	 */
	    if ((pl <= cbl) || strncmp(cb, pp, cbl))
		return((char *)NULL);
	    pp += cbl;
	    pl -= cbl;
	    if (cb[cbl - 1] == '/') {

	    /*
	     * The CWD ends in a '/', so the path must not begin with one.  If
	     * it does, no trimming can be done.
	     */
		if (*pp == '/')
		    return((char *)NULL);
	    } else {

	    /*
	     * The CWD doesn't end in a '/', so the path must begin with one.
	     * If it doesn't, no trimming can be done.
	     */
		if (*pp != '/')
		    return((char *)NULL);
	    /*
	     * Skip all leading path '/' characters.  Some characters must
	     * remain.
	     */
		while ((pl > 0) && (*pp == '/')) {
		    pp++;
		    pl--;
		}
		if (!pl)
		    return((char *)NULL);
	    }
	}
/*
 * Allocate space for the assembled path, including terminator, and return its
 * pointer.
 */

getvpath_alloc:

	if (!(ap = (char *)malloc(pl + 1))) {
	    (void) fprintf(stderr, "%s: no getvpath space (%d)\n",
		Pn, pl + 1);
	    Exit(1);
	}
	(void) memmove(ap, pp, pl + 1);
	return(ap);
}
#endif	/* DARWINV>=800 */


#if	DARWINV<600
/*
 * lkup_dev_tty() - look up /dev/tty
 */

static int
lkup_dev_tty(dr, rdr, ir)
	dev_t *dr;			/* place to return device number */
	dev_t *rdr;			/* place to return raw device number */
	INODETYPE *ir;			/* place to return inode number */
{
	int i;

	readdev(0);
	for (i = 0; i < Ndev; i++) {
	    if (strcmp(Devtp[i].name, "/dev/tty") == 0) {
		*dr = DevDev;
		*rdr = Devtp[i].rdev;
		*ir = (INODETYPE)Devtp[i].inode;
		return(1);
	    }
	}
	return(-1);
}
#endif	/* DARWINV<600 */


/*
 * process_node() - process vnode
 */

void
process_node(va)
	KA_T va;			/* vnode kernel space address */
{
	dev_t dev = (dev_t)0;
	dev_t rdev = (dev_t)0;
	unsigned char devs = 0;
	unsigned char rdevs = 0;

#if	DARWINV<800
	struct devnode *d = (struct devnode *)NULL;
	struct devnode db;
	unsigned char lt;
	char dev_ch[32];

# if	defined(HASFDESCFS)
	struct fdescnode *f = (struct fdescnode *)NULL;
	struct fdescnode fb;
# endif	/* defined(HASFDESCFS) */

	static INODETYPE fi;
	static dev_t fdev, frdev;
	static int fs = 0;
	struct inode *i = (struct inode *)NULL;
	struct inode ib;
	struct lockf lf, *lff, *lfp;
	struct nfsnode *n = (struct nfsnode *)NULL;
	struct nfsnode nb;
#else	/* DARWINV>=800 */
	struct stat sb;
	char *vn;
#endif	/* DARWINV<800 */

	char *ty;
	enum vtype type;
	struct vnode *v, vb;
	struct l_vfs *vfs;

#if	DARWINV<600
	struct hfsnode *h = (struct hfsnode *)NULL;
	struct hfsnode hb;
	struct hfsfilemeta *hm = (struct hfsfilemeta *)NULL;
	struct hfsfilemeta hmb;
#else	/* DARWINV>=600 */
# if	DARWINV<800
	struct cnode *h = (struct cnode *)NULL;
	struct cnode hb;
	struct filefork *hf = (struct filefork *)NULL;
	struct filefork hfb;
# endif	/* DARWINV<800 */
#endif	/* DARWINV<600 */

#if	defined(HAS9660FS)
	dev_t iso_dev;
	int iso_dev_def = 0;
	INODETYPE iso_ino;
	long iso_links;
	int iso_stat = 0;
	SZOFFTYPE iso_sz;
#endif	/* defined(HAS9660FS) */

/*
 * Read the vnode.
 */
	if ( ! va) {
	    enter_nm("no vnode address");
	    return;
	}
	v = &vb;
	if (readvnode(va, v)) {
	    enter_nm(Namech);
	    return;
	}
	type = v->v_type;

#if	defined(HASNCACHE)
	Lf->na = va;
# if	defined(HASNCVPID)
	Lf->id = v->v_id;
# endif	/* defined(HASNCVPID) */
#endif	/* defined(HASNCACHE) */

#if	defined(HASFSTRUCT)
	Lf->fna = va;
	Lf->fsv |= FSV_NI;
#endif	/* defined(HASFSTRUCT) */

/*
 * Get the vnode type.
 */
	if (!v->v_mount)
	    vfs = (struct l_vfs *)NULL;
	else {
	    vfs = readvfs((KA_T)v->v_mount);
	    if (vfs) {
		if (strcasecmp(vfs->typnm, "nfs") == 0)
		    Ntype = N_NFS;

#if	DARWINV<130
		else if (strcasecmp(vfs->typnm, "afpfs") == 0)
		    Ntype = N_AFPFS;
#endif	/* DARWINV<130 */

	    }
	}
	if (Ntype == N_REGLR) {
	    switch (v->v_type) {
	    case VFIFO:
		Ntype = N_FIFO;
		break;
	    default:
		break;
	    }
	}

#if	DARWINV<800
/*
 * Define the specific node pointer.
 */
	switch (v->v_tag) {

# if	DARWINV>120
	case VT_AFP:
 	    break;
# endif	/* DARWINV>120 */

# if	DARWINV>120
	case VT_CDDA:
	    break;
# endif	/* DARWINV>120 */

# if	DARWINV>120
	case VT_CIFS:
	    break;
# endif	/* DARWINV>120 */

	case VT_DEVFS:
	    if (!v->v_data
	    ||  kread((KA_T)v->v_data, (char *)&db, sizeof(db))) {
		(void) snpf(Namech, Namechl, "no devfs node: %#x", v->v_data);
		enter_nm(Namech);
		return;
	    }
	    d = &db;
	    break;

# if	defined(HASFDESCFS)
	case VT_FDESC:
	    if (!v->v_data
	    ||  kread((KA_T)v->v_data, (char *)&fb, sizeof(fb))) {
		(void) snpf(Namech, Namechl, "no fdesc node: %s",
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    f = &fb;
	    break;
# endif	/* defined(HASFDESCFS) */

	case VT_HFS:

# if	DARWINV<130
	    if (Ntype != N_AFPFS) {
# endif	/* DARWINV<130 */

		if (!v->v_data
		||  kread((KA_T)v->v_data, (char *)&hb, sizeof(hb))) {
		    (void) snpf(Namech, Namechl, "no hfs node: %s",
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		h = &hb;

# if	DARWINV<600
		if (!h->h_meta
		||  kread((KA_T)h->h_meta, (char *)&hmb, sizeof(hmb))) {
		    (void) snpf(Namech, Namechl, "no hfs node metadata: %s",
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		hm = &hmb;
# else	/* DARWINV>=600 */
		if (v->v_type == VDIR)
		    break;
		if (h->c_rsrc_vp == v)
		    hf = h->c_rsrcfork;
		else
		    hf = h->c_datafork;
		if (!hf
		||  kread((KA_T)hf, (char *)&hfb, sizeof(hfb))) {
		    (void) snpf(Namech, Namechl, "no hfs node fork: %s",
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		hf = &hfb;
# endif	/* DARWINV<600 */

# if	DARWINV<130
	    }
# endif	/* DARWINV<130 */

	    break;

# if	defined(HAS9660FS)
	case VT_ISOFS:
	    if (read_iso_node(v, &iso_dev, &iso_dev_def, &iso_ino, &iso_links,
			      &iso_sz))
	    {
		(void) snpf(Namech, Namechl, "no iso node: %s",
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    iso_stat = 1;
	    break;
# endif	/* defined(HAS9660FS) */

	case VT_NFS:
	    if (!v->v_data
	    ||  kread((KA_T)v->v_data, (char *)&nb, sizeof(nb))) {
		(void) snpf(Namech, Namechl, "no nfs node: %s",
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    n = &nb;
	    break;

# if	DARWINV>120
	case VT_UDF:
	    break;
# endif	/* DARWINV>120 */

	case VT_UFS:
	    if (!v->v_data
	    ||  kread((KA_T)v->v_data, (char *)&ib, sizeof(ib))) {
		(void) snpf(Namech, Namechl, "no ufs node: %s",
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    i = &ib;
	    if ((lff = i->i_lockf)) {

	    /*
	     * Determine the lock state.
	     */
		lfp = lff;
		do {
		    if (kread((KA_T)lfp, (char *)&lf, sizeof(lf)))
			break;
		    lt = 0;
		    switch (lf.lf_flags & (F_FLOCK|F_POSIX)) {
		    case F_FLOCK:
			if (Cfp && (struct file *)lf.lf_id == Cfp)
			    lt = 1;
			break;
		    case F_POSIX:
			if ((KA_T)lf.lf_id == Kpa)
			    lt = 1;
			break;
		    }
		    if (!lt)
			continue;
		    if (lf.lf_start == (off_t)0
		    &&  lf.lf_end == 0xffffffffffffffffLL)
			lt = 1;
		    else
			lt = 0;
		    if (lf.lf_type == F_RDLCK)
			Lf->lock = lt ? 'R' : 'r';
		    else if (lf.lf_type == F_WRLCK)
			Lf->lock = lt ? 'W' : 'w';
		    else if (lf.lf_type == (F_RDLCK | F_WRLCK))
			Lf->lock = 'u';
		    break;
		} while ((lfp = lf.lf_next) && lfp != lff);
	    }
	    break;

# if	DARWINV>120
	case VT_WEBDAV:
   	    break;
# endif	/* DARWINV>120 */

	default:
	    if (v->v_type == VBAD || v->v_type == VNON)
		break;
	    (void) snpf(Namech, Namechl, "unknown file system type: %d",
		v->v_tag);
	    enter_nm(Namech);
	    return;
	}
/*
 * Get device and type for printing.
 */
	if (n) {
	    dev = n->n_vattr.va_fsid;
	    devs = 1;
	} else if (i) {
	    dev = i->i_dev;
	    devs = 1;
	    if ((type == VCHR) || (type == VBLK)) {
		rdev = i->i_rdev ;
		rdevs = 1;
	    }
	}

# if	defined(HASFDESCFS)
	else if (f) {
	    if (f->fd_link
	    &&  !kread((KA_T)f->fd_link, Namech, Namechl -1))
		Namech[Namechl - 1] = '\0';

#  if	DARWINV<600
	    else if (f->fd_type == Fctty) {
		if (fs == 0)
		    fs = lkup_dev_tty(&fdev, &frdev, &fi);
		if (fs == 1) {
		    dev = fdev;
		    rdev = frdev;
		    devs = Lf->inp_ty = rdevs = 1;
		    Lf->inode = fi;
		}
	    }
	}
#  endif	/* DARWINV<600 */
# endif	/* defined(HASFDESCFS) */

	else if (h) {

# if	DARWINV<600
	    dev = hm->h_dev;
# else	/* DARWINV>=600 */
	    dev = h->c_dev;
# endif	/* DARWINV<600 */

	    devs = 1;
	    if ((type == VCHR) || (type == VBLK)) {

# if	DARWINV<600
		rdev = hm->h_rdev;
# else	/* DARWINV>=600 */
		rdev = h->c_rdev;
# endif	/* DARWINV<600 */

		rdevs = 1;
	    }
	} else if (d) {
	    dev = DevDev;
	    devs = 1;
	    rdev = d->dn_typeinfo.dev;
	    rdevs = 1;
	}

# if	defined(HAS9660FS)
	else if (iso_stat && iso_dev_def) {
	    dev = iso_dev;
	    devs = Lf->inp_ty = 1;
	}
# endif	/* defined(HAS9660FS) */


/*
 * Obtain the inode number.
 */
	if (i) {
	    Lf->inode = (INODETYPE)i->i_number;
	    Lf->inp_ty = 1;
	} else if (n) {
	    Lf->inode = (INODETYPE)n->n_vattr.va_fileid;
	    Lf->inp_ty = 1;
	} else if (h) {

# if	DARWINV<600
	    Lf->inode = (INODETYPE)hm->h_nodeID;
# else	/* DARWINV>=600 */
	    Lf->inode = (INODETYPE)h->c_fileid;
# endif	/* DARWINV<600 */

	    Lf->inp_ty = 1;
	}

# if	defined(HAS9660FS)
	else if (iso_stat) {
	    Lf->inode = iso_ino;
	    Lf->inp_ty = 1;
	}
# endif	/* defined(HAS9660FS) */

/*
 * Obtain the file size.
 */
	if (Foffset)
	    Lf->off_def = 1;
	else {
	    switch (Ntype) {
	    case N_FIFO:
		if (!Fsize)
		    Lf->off_def = 1;
		break;
	    case N_NFS:
		if (n) {
		    Lf->sz = (SZOFFTYPE)n->n_vattr.va_size;
		    Lf->sz_def = 1;
		}
		break;

# if	DARWINV<130
	    case N_AFPFS:
		break;
# endif	/* DARWINV<130 */

	    case N_REGLR:
		if (type == VREG || type == VDIR) {
		    if (i) {
			Lf->sz = (SZOFFTYPE)i->i_size;
			Lf->sz_def = 1;
		    } else if (h) {

# if	DARWINV<600
			Lf->sz = (type == VDIR) ? (SZOFFTYPE)hm->h_size
						: (SZOFFTYPE)h->fcbEOF;
# else	/* DARWINV>=600 */
			if (type == VDIR)
			    Lf->sz = (SZOFFTYPE)h->c_nlink * 128;
			else
			    Lf->sz = (SZOFFTYPE)hf->ff_size;
# endif	/* DARWINV<600 */

			Lf->sz_def = 1;
		    }

# if	defined(HAS9660FS)
		    else if (iso_stat) {
			Lf->sz = (SZOFFTYPE)iso_sz;
			Lf->sz_def = 1;
		    }
# endif	/* defined(HAS9660FS) */

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
	    switch(Ntype) {
	    case N_NFS:
		if (n) {
		    Lf->nlink = (long)n->n_vattr.va_nlink;
		    Lf->nlink_def = 1;
		}
		break;

# if	DARWINV<130
	    case N_AFPFS:
		break;
# endif	/* DARWINV<130 */

	    case N_REGLR:
		if (i) {
		    Lf->nlink = (long)i->i_nlink;
		    Lf->nlink_def = 1;
		} else if (h) {

# if	DARWINV<600
		    Lf->nlink = (long)hm->h_nlink;
# else	/* DARWINV>=600 */
		    Lf->nlink = (long)h->c_nlink;
# endif	/* DARWINV<600 */

		    Lf->nlink_def = 1;
		}

# if	defined(HAS9660FS)
		else if (iso_stat) {
		    Lf->nlink = iso_links;
		    Lf->nlink_def = 1;
		}
# endif	/* defined(HAS9660FS) */

		break;
	    }
	    if (Lf->nlink_def && Nlink && (Lf->nlink < Nlink))
		Lf->sf |= SELNLINK;
	}
#else	/* DARWINV>=800 */

/*
 * Process a vnode for Darwin >= 8.0.
 */
	if ((vn = getvpath(va, v))) {

	/*
	 * If the vnode yields a path, get the file's information by doing
	 * a "safe" stat(2) of the path.
	 */
	    if (!statsafely(vn, &sb)) {

	    /*
	     * Save file size or offset.
	     */
		if (Foffset) {
		    Lf->off_def = 1;
		} else {
		    switch (Ntype) {
		    case N_FIFO:
			if (!Fsize)
			    Lf->off_def = 1;
			break;
		    case N_NFS:
		    case N_REGLR:
			if (type == VREG || type == VDIR) {
			    Lf->sz = sb.st_size;
			    Lf->sz_def = 1;
			} else if ((type == VCHR || type == VBLK) && !Fsize)
			    Lf->off_def = 1;
			break;
		    }
		}
	    /*
	     * Save node number.
	     */
		Lf->inode = (INODETYPE)sb.st_ino;
		Lf->inp_ty = 1; 
	    /*
	     * Optionally save link count.
	     */
		if (Fnlink) {
		    Lf->nlink = sb.st_nlink;
		    Lf->nlink_def = 1;
		}
	    /*
	     * Save device number and path.
	     */
		switch (v->v_tag) {
		case VT_DEVFS:
		    if (vn)
			(void) free((FREE_P *)vn);
		    dev = DevDev;
		    devs = 1;
		    break;
		default :
		    Lf->V_path = vn;
		    dev = sb.st_dev;
		    devs = 1;
		    break;
		}
	    /*
	     * Save character and block device number.
	     */
		if ((type == VCHR) || (type == VBLK)) {
		    rdev = sb.st_rdev;
		    rdevs = 1;
		}
	    } else {

	    /*
	     * Indicate a stat(2) failure in Namech[].
	     */
		(void) snpf(Namech, Namechl, "stat(%s): %s", vn,
		    strerror(errno));
		(void) free((FREE_P *)vn);
	    }
	/*
	 * Record an NFS file.
	 */
	    if (vfs && !strcmp(vfs->typnm, "nfs"))
		Ntype = N_NFS;
	}
#endif	/* DARWINV>=800 */

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
	    ty = "VREG";
	    break;
	case VDIR:
	    ty = "VDIR";
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
	    break;
	default:
	    (void) snpf(Lf->type, sizeof(Lf->type), "%04o", (type & 0xfff));
	    ty = (char *)NULL;
	}
	if (ty)
	    (void) snpf(Lf->type, sizeof(Lf->type), "%s", ty);
	Lf->ntype = Ntype;
/*
 * Handle some special cases:
 *
 * 	ioctl(fd, TIOCNOTTY) files;
 *	memory node files;
 *	/proc files.
 */
	if (type == VBAD)
	    (void) snpf(Namech, Namechl, "(revoked)");

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
				   ((type == VCHR) || (type == VBLK) ? 1
								     : 0)))
	    Lf->sf |= SELNM;
/*
 * Enter name characters.
 */
	if (Namech[0])
	    enter_nm(Namech);
}


#if	DARWINV>=800
/*
 * readvname() - read vnode's path name
 */

static int
readvname(addr, buf, buflen)
	KA_T addr;			/* kernel v_path address */
	char *buf;			/* receiving buffer */
	int buflen;			/* sizeof(buf) */
{
	int n, rl;
/*
 * Read the name 32 characters at a time, until a NUL character
 * has been read or the buffer has been filled.
 */
	for (n = 0; n < buflen; addr += 32, n += 32) {
	    rl = buflen - n;
	    if (rl > 32)
		rl = 32;
	    if (kread(addr, &buf[n], rl))
		return(0);
	    buf[n + rl] = '\0';
	    if ((rl = (int)strlen(&buf[n])) < 32) {
		return(n + rl);
	    }
	}
	return(0);
}
#endif	/* DARWINV>=800 */
