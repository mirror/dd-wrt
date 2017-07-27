/*
 * dfile.c -- pstat-based HP-UX file functions for lsof
 */


/*
 * Copyright 1999 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1999 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id";
#endif


#include "lsof.h"


#if	defined(HASNCACHE)
/*
 * Local definitions
 */

#define DNLCINCR	2048		/* DNLC read increment */

#define	NFSIDH		256		/* file system ID hash count
					 * -- MUST BE A POWER OF TWO!!! */
#define	NFSID		sizeof(struct psfsid)
					/* size of psfsid structure */
#define	NL_NC		sizeof(struct l_nc)
					/* size of l_nc structure */
#define	NPSTM		sizeof(struct pst_mpathnode)
					/* size of pst_mpathnode */


/*
 * Local structure definitions
 */

struct l_nc {				/* local name cache */
	struct psfileid id;		/* node's PSTAT ID */
	struct psfileid par;		/* parent's PSTAT ID */
	struct l_nc *pl;		/* local parent name cache entry --
					 * NULL if not found or not yet
					 * accessed (see pls) */
	int pls;			/* status of pl: 0 = not accessed
					 *		 1 = accessed */
	int nl;				/* name length */
	char *nm;			/* name */
	struct l_nc *next;		/* next hash bucket link */
};

struct l_fic {				/* file system ID cache */
	struct psfsid fsid;		/* file system ID */
	int nc;				/* files cached for file system */
	struct l_fic *next;		/* next hash bucket link */
};


/*
 * Local static variables
 */

static int Nceh;			/* number of Nchash entries allocated */
static struct l_nc **Nchash = (struct l_nc **)NULL;
					/* the name cache hash buckets */
static int Ncmask;			/* power of two mask for the name
					 * cache -- sized from Nc */
static int Ndnlc;			/* number of DNLC entries via
					 * pst_dynamic.psd_dnlc_size */
static struct l_fic **Ncfsid = (struct l_fic **)NULL;
					/* the file system hash buckets */
static struct pst_fid Nzpf;		/* zeroed pst_fid (for memcmp()) */
static struct psfileid Nzps;		/* zeroed psfilid (for memcmp()) */
static int Nzpfs = 0;			/* Nzpf status: 1 = zeroed */
static int Nzpss = 0;			/* Nzps status: 1 = zeroed */


/*
 * Local macros
 */

#define	HASHFSID(i)	(Ncfsid + \
	(((int)(((((struct psfsid *)i)->psfs_id * 31415) << 3)&0xfffffff) \
	+ (int)((((((struct psfsid *)i)->psfs_type * 31415) << 5)&0xfffffff))) \
	& (NFSIDH - 1)))
#define	HASHPSFILEID(p)	(Nchash + \
	(((int)(((int)((((struct psfileid *)p)->psf_fsid.psfs_id * 31415) << 3)\
		& 0xfffffff) \
	+ (int)(((((struct psfileid *)p)->psf_fsid.psfs_type * 31415) << 5) \
		& 0xfffffff) \
	+ (int)(((((struct psfileid *)p)->psf_fileid * 31415) << 7) \
		& 0xfffffff))) \
	& Ncmask))


/*
 * Local function prototypes
 */

_PROTOTYPE(static struct l_nc *ncache_addr,(struct psfileid *ps));
_PROTOTYPE(static void ncache_free,(void));
_PROTOTYPE(static int ncache_isroot,(struct psfileid *ps));
_PROTOTYPE(static void ncache_size,(void));
#endif	/* defined(HASNCACHE) */


#if	defined(HASIPv6)
/*
 * gethostbyname2() -- an RFC2133-compatible get-host-by-name-two function
 *                     to get AF_INET and AF_INET6 addresses from host names,
 *                     using the gethostbyname() and RFC2553-compatible
 *		       getipnodebyname() functions
 */

extern struct hostent *
gethostbyname2(nm, prot)
	char *nm; 			/* host name */
	int prot;			/* protocol -- AF_INET or AF_INET6 */
{
	int err;

	if (prot == AF_INET) {

	/*
	 * This shouldn't be necessary if /etc/nsswitch.conf is correct, but
	 * it's a good fail-safe in case /etc/nsswitch.conf is missing or
	 * incorrect.
	 */
	    return(gethostbyname(nm));
	}
	return(getipnodebyname(nm, prot, 0, &err));
}
#endif	/* defined(HASIPv6) */


/*
 * get_max_fd() -- get maximum file descriptor plus one
 */

int
get_max_fd()
{
	struct rlimit r;

	if (getrlimit(RLIMIT_NOFILE, &r))
	    return(-1);
	return(r.rlim_cur);
}


#if	defined(HASNCACHE)


/*
 * ncache_addr() -- get ncache entry address
 */

static struct l_nc *
ncache_addr(ps)
	struct psfileid *ps;		/* parent's psfileid */
{
	struct l_nc **hp, *lc;

	for (hp = HASHPSFILEID(ps), lc = *hp; lc; lc = lc->next) {
	    if (!memcmp((void *)ps, (void *)&lc->id, sizeof(struct psfileid)))
		return(lc);
	}
	return((struct l_nc *)NULL);
}


/*
 * ncache_alloc() -- allocate name cache space
 */

static void
ncache_alloc()
{
	if (Nchash || Ncfsid)
	    ncache_free();
	(void) ncache_size();
	if (!(Nchash = (struct l_nc **)calloc(Nceh, sizeof(struct l_nc *))))
	{
	    (void) fprintf(stderr,
		"%s: can't allocate %d local name cache entries\n", Pn, Nceh);
	    Exit(1);
	}
	if (Ncfsid)
	    return;
	if (!(Ncfsid = (struct l_fic **)calloc(NFSIDH, sizeof(struct l_fic *))))
	{
	    (void) fprintf(stderr,
		"%s: can't allocate %d local file system cache entries\n",
		Pn, NFSIDH);
	    Exit(1);
	}
}


/*
 * ncache_free() -- free previous ncache allocations
 */

static void
ncache_free()
{
	int i;
	struct l_fic **fh, *fp, *fx;
	struct l_nc **nh, *np, *nx;

	if (Ncfsid) {

	/*
	 * Free file system ID hash bucket contents.
	 */
	    for (fh = Ncfsid, i = 0; i < NFSIDH; fh++, i++) {
		for (fp = *fh; fp; fp = fx) {
		    fx = fp->next;
		    (void) free((MALLOC_P *)fp);
		}
		Ncfsid[i] = (struct l_fic *)NULL;
	    }
	}
	if (Nchash) {

	/*
	 * Free name cache.
	 */
	    for (i = 0, nh = Nchash; i < Nceh; i++, nh++) {
		for (np = *nh; np; np = nx) {
		    nx = np->next;
		    if (np->nm)
			(void) free((MALLOC_P *)np->nm);
		    (void) free((MALLOC_P *)np);
		}
	    }
	    (void) free((MALLOC_P *)Nchash);
	    Nchash = (struct l_nc **)NULL;
	}
}


/*
 * ncache_isroot() -- does psfileid represent the root of a file system?
 */

static int
ncache_isroot(ps)
	struct psfileid *ps;		/* psfileid */
{
	if (!ps->psf_fsid.psfs_id && !ps->psf_fsid.psfs_type
	&&  ps->psf_fileid == -1)
	    return(1);

# if	defined(HASFSINO)
	if (!Lf->fs_ino || (Lf->inp_ty != 1) || !Lf->dev_def)
	    return(0);
	if ((Lf->dev == (dev_t)ps->psf_fsid.psfs_id)
	&&  (Lf->fs_ino == (unsigned long)ps->psf_fileid))
	    return(1);
# endif	/* defined(HASFSINO) */

	return(0);
}


/*
 * ncache_load() -- load name cache
 */

void
ncache_load()
{
	if (!Fncache)
	    return;
	(void) ncache_alloc();
	if (!Nzpfs) {
	    (void)memset((void *)&Nzpf, 0, sizeof(Nzpf));
	    Nzpfs = 1;
	}
	if (!Nzpss) {
	    (void)memset((void *)&Nzps, 0, sizeof(Nzps));
	    Nzpss = 1;
	}
}


/*
 * ncache_loadfs() -- load the name cache for a file system
 */

struct l_fic *
ncache_loadfs(fsid, fh)
	struct psfsid *fsid;		/* ID of file system to add */
	struct l_fic **fh;		/* Ncfsid hash bucket */
{
	char *cp;
	struct l_fic *f;
	int i, nl, nr;
	struct pst_mpathnode mp[DNLCINCR];
	struct l_nc **nh, *nn, *nt, *ntp;
	int x = 0;
/*
 * Allocate a new file system pointer structure and link it to its bucket.
 */
	if (!(f = (struct l_fic *)malloc(sizeof(struct l_fic)))) {
	    (void) fprintf(stderr, "%s: no fsid structure space\n", Pn);
	    Exit(1);
	}
	f->fsid = *fsid;
	f->nc = 0;
	f->next = *fh;
	*fh = f;
	while ((nr = pstat_getmpathname(&mp[0], NPSTM, DNLCINCR, x, fsid)) > 0)
	{
	    x = mp[nr - 1].psr_idx + 1;
	    for (i = 0; i < nr; i++) {

	    /*
	     * Ignore NUL names, ".", and "..".
	     */
		if (!(nl = (int)strlen(mp[i].psr_name)))
		    continue;
		if ((nl < 3) && (mp[i].psr_name[0] == '.')) {
		    if ((nl == 1) || (mp[i].psr_name[1] == '.'))
			continue;
		}
	    /*
	     * Allocate name and name cache structure space.
	     */
		if (!(cp = (char *)malloc((MALLOC_S)(nl + 1)))) {
		    (void) fprintf(stderr,
			"%s: no name entry space (%d) for:%s\n",
			Pn, nl + 1, mp[i].psr_name);
		    Exit(1);
		}
		if (!(nn = (struct l_nc *)malloc(sizeof(struct l_nc)))) {
		    (void) fprintf(stderr,
			"%s: no name cache entry space (%d) for: %s\n",
			Pn, (int)sizeof(struct l_nc), mp[i].psr_name);
		    Exit(1);
		}
	    /*
	     * Fill in name cache entry, complete with name and name length.
	     */
		(void) snpf(cp, nl + 1, "%s", mp[i].psr_name);
		nn->id = mp[i].psr_file;
		nn->par = mp[i].psr_parent;
		nn->nm = cp;
		nn->nl = nl;
		nn->pl = nn->next = (struct l_nc *)NULL;
		nn->pls = 0;
		nh = HASHPSFILEID(&mp[i].psr_file);
	    /*
	     * Skip to the end of the hash bucket chain, looking for
	     * duplicates along the way.
	     */
		for (nt = *nh, ntp = (struct l_nc *)NULL;
		     nt;
		     ntp = nt, nt = nt->next)
		{
		    if (memcmp((void *)&nt->id, (void *)&nn->id, NL_NC) == 0)
			break;
		}
		if (nt) {

		/*
		 * Remove a duplicate.
		 */
		    if (ntp)
			ntp = nt->next;
		    else
			*nh = nt->next;
		    (void) free((MALLOC_P *)nt->nm);
		    (void) free((MALLOC_P *)nt);
		    (void) free((MALLOC_P *)nn->nm);
		    (void) free((MALLOC_P *)nn);
		} else {

		/*
		 * Link a new entry.
		 */
		    if (ntp)
			ntp->next = nn;
		    else
			*nh = nn;
		    f->nc++;
		}
	    }
	    if (nr < DNLCINCR)
		break;
	}
	return(f);
}


/*
 * ncache_lookup() -- look up a node's name in the kernel's name cache
 */

char *
ncache_lookup(buf, blen, fp)
	char *buf;			/* receiving name buffer */
	int blen;			/* receiving buffer length */
	int *fp;			/* full path reply */
{
	char *cp = buf;
	int ef;
	struct l_fic **fh, *fs;
	struct l_nc *lc;
	int nl, rlen;
	char *pc;

	*cp = '\0';
	*fp = 0;

# if	defined(HASFSINO)
/*
 * If the entry has an inode number that matches the inode number of the
 * file system mount point, return an empty path reply.  That tells the
 * caller that the already-printed system mount point name is sufficient.
 */
	if (Lf->inp_ty == 1 && Lf->fs_ino && Lf->inode == Lf->fs_ino)
	    return(cp);
# endif	/* defined(HASFSINO) */

/*
 * See if cache has been loaded for this pfsid.  Don't try to load if cache
 * loading has been inhibited with -C, or unless the real or effective UID of
 * this process is root.
 */
	if ((!Myuid || Setuidroot) && Fncache) {
	    for (fh = HASHFSID(&Lf->psfid.psf_fsid), fs = *fh;
		 fs;
		 fs = fs->next)
	    {
		if (memcmp((void *)&fs->fsid, (void *)&Lf->psfid.psf_fsid,
			    NFSID)
		== 0)
		    break;
	    }
	    if (!fs)
		fs = ncache_loadfs(&Lf->psfid.psf_fsid, fh);
	} else
	    fs = (struct l_fic *)NULL;
/*
 * Search the cache for an entry whose psfileid matches.
 */
	if (!fs || !fs->nc || !(lc = ncache_addr(&Lf->psfid))) {

	/*
	 * If the node has no cache entry, see if it's the root of the file
	 * system.
	 */

# if	defined(HASFSINO)
	    if (Lf->fs_ino && (Lf->inp_ty == 1) && (Lf->fs_ino == Lf->inode))
		return(cp);
# endif	/* defined(HASFSINO) */

	/*
	 * If the file system's cache couldn't be loaded -- e.g., this lsof
	 * process lacks permission to load it or cache lookup is inhibited
	 * with -C -- but the UID of the file's process matches the UID of the
	 * lsof process, see if it's possible to read the single path name for
	 * this particular file.  (The file must have a non-zero opaque ID.)
	 */
	    if (!fs) {
		if (Fncache
		&&  (Myuid == Lp->uid)
		&&  memcmp((void *)&Lf->opfid, (void *)&Nzpf, sizeof(Nzpf))
		&&  (nl = pstat_getpathname(buf, (blen - 1), &Lf->opfid)) > 0)
		{
		    buf[nl] = '\0';
		    if (*buf == '/')
			*fp = 1;
		    return(buf);
		}
	    }
	    return((char *)NULL);
	}
	if (ncache_isroot(&lc->id)) {

	/*
	 * If the node is the root of the file system, return a response
	 * that will cause the root directory to be displayed.
	 */
	    return(cp);
	}
/*
 * Start the path assembly.
 */
	if ((nl = lc->nl) > (blen - 1))
	    return((char *)NULL);
	cp = buf + blen - nl - 1;
	rlen = blen - nl - 1;
	(void) snpf(cp, nl + 1, "%s", lc->nm);
/*
 * Look up the name cache entries that are parents of the node address.
 * Quit when:
 *
 *	there's no parent;
 *	the file system root is reached;
 *	the name length is too large to fit in the receiving buffer.
 */
	for (ef = 0; !ef;) {
	    if (!lc->pl) {
		if (!lc->pls) {

		/*
		 * If there is a parent, look up its Ncache address;
		 * otherwise quit on an incomplete path assembly.
		 */
		    if (memcmp((void *)&lc->par, (void *)&Nzps, sizeof(Nzps))) {
			lc->pl = ncache_addr(&lc->par);
			lc->pls = 1;
		    } else
			break;
		}
	    }
	    if (ncache_isroot(&lc->par)) {

	    /*
	     * If the parent entry is the file system root, enter the file
	     * system root directory, and indicate that the assembly should
	     * stop after this entry.
	     */
		if (!(pc = Lf->fsdir))
		    break;
		nl = (int)strlen(pc);
		ef = 1;
	    } else {

	    /*
	     * Use the parent link if it exists; otherwise exit on an
	     * incomplete path assembly.
	     */
	        if (!(lc = lc->pl))
		    break;
		pc = lc->nm;
		nl = lc->nl;
	    }
	/*
	 * Prefix the next path component.  Intersperse a '/' if the
	 * component doesn't end in one.
	 */
	    if (!nl)
		break;
	    if (pc[nl - 1] != '/') {
		if (1 > rlen)
		    break;
		*(cp - 1) = '/';
		cp--;
		rlen--;
	    }
	    if (nl > rlen)
		break;
	    (void) strncpy((cp - nl), pc, nl);
	    cp -= nl;
	    rlen -= nl;
	    if (ef) {

	    /*
	     * If the file system root directory was just prefixed, return
	     * a full-path indication.
	     */
		*fp = 1;
		break;
	    }
	}
	return(cp);
}


/*
 * ncache_size() -- get DNLC size
 */

static void
ncache_size()
{
	struct pst_dynamic pd;

	if (pstat_getdynamic(&pd, sizeof(pd), 1, 0) != 1) {
	    (void) fprintf(stderr, "%s: can't get dynamic status\n", Pn);
	    Exit(1);
	}
	Ndnlc = (int)pd.psd_dnlc_size;
	for (Nceh = 1; Nceh < (Ndnlc + Ndnlc); Nceh <<= 1)
		;
	Ncmask = Nceh - 1;
}
#endif	/* defined(HASNCACHE) */


/*
 * print_dev() -- print device
 */

char *
print_dev(lf, dev)
	struct lfile *lf;		/* file whose device is to be printed */
	dev_t *dev;			/* device to be printed */
{
	static char buf[128];

	(void) snpf(buf, sizeof(buf), "%d,%#x", GET_MAJ_DEV(*dev),
		    GET_MIN_DEV(*dev));
	return(buf);
}


/*
 * process_finfo() -- process file information
 */

void
process_finfo(pd, opfid, psfid, na)
	struct pst_filedetails *pd;	/* file details */
	struct pst_fid *opfid;		/* opaque file ID for this file */
	struct psfileid *psfid;		/* PSTAT file ID for this file */
	KA_T na;			/* node address */
{
	char *cp, buf[32];
	dev_t dev;
	int devs = 0;
	int32_t lk;
	struct mounts *mp;
/*
 * Save file IDs for later use in name lookup.
 */
	Lf->opfid = *opfid;
	Lf->psfid = *psfid;

#if	defined(HASFSTRUCT)
/*
 * Save node ID.
 */
	if (na && (Fsv & FSV_NI)) {
	    Lf->fna = na;
	    Lf->fsv |= FSV_NI;
	}
#endif	/* defined(HASFSTRUCT) */

/*
 * Construct lock code.
 */
	if ((lk = pd->psfd_lckflag) & PS_FPARTRDLCK)
	    Lf->lock = 'r';
	else if (lk & PS_FPARTWRLCK)
	    Lf->lock = 'w';
	else if (lk & PS_FFULLRDLCK)
	    Lf->lock = 'R';
	else if (lk & PS_FFULLWRLCK)
	    Lf->lock = 'W';
	else
	    Lf->lock = ' ';
/*
 * Derive type from modes.
 */
	switch ((int)(pd->psfd_mode & PS_IFMT)) {
	case PS_IFREG:
	    cp = "REG";
	    Ntype = N_REGLR;
	    break;
	case PS_IFBLK:
	    cp = "BLK";
	    Ntype = N_BLK;
	    break;
	case PS_IFDIR:
	    cp = "DIR";
	    Ntype = N_REGLR;
	    break;
	case PS_IFCHR:
	    cp = "CHR";
	    Ntype = N_CHR;
	    break;
	case PS_IFIFO:
	    cp = "FIFO";
	    Ntype = N_FIFO;
	    break;
	default:
	    (void) snpf(buf, sizeof(buf), "%04o",
		(unsigned int)(((pd->psfd_mode & PS_IFMT) >> 12) & 0xfff));
	    cp = buf;
	    Ntype = N_REGLR;
	}
	if (!Lf->type[0])
	    (void) snpf(Lf->type, sizeof(Lf->type), "%s", cp);
	Lf->ntype = Ntype;
/*
 * Save device number.
 */
	switch (Ntype) {
	case N_FIFO:
	    (void) enter_dev_ch(print_kptr(na, (char *)NULL, 0));
	    break;
	default:
	    dev = Lf->dev = (dev_t)pd->psfd_dev;
	    devs = Lf->dev_def = 1;
	    if ((Ntype == N_CHR) || (Ntype == N_BLK)) {
		Lf->rdev = (dev_t)pd->psfd_rdev;
		Lf->rdev_def = 1;
	    }
	}
/*
 * Save node number.
 */
	Lf->inode = (INODETYPE)pd->psfd_ino;
	Lf->inp_ty = 1;
/*
 * Save link count.
 */
	if (Fnlink) {

	/*
	 * Ignore a zero link count only if the file is a FIFO.
	 */
	    if ((Lf->nlink = (long)pd->psfd_nlink) || (Ntype != N_FIFO))
		Lf->nlink_def = 1;
	    if (Lf->nlink_def && Nlink && (Lf->nlink < Nlink))
		Lf->sf |= SELNLINK;
	}
/*
 * Save file system identity.
 */
	if (devs) {
	    for (mp = readmnt(); mp; mp = mp->next) {
		if (dev == mp->dev) {
		    Lf->fsdir = mp->dir;
		    Lf->fsdev = mp->fsname;

#if	defined(HASFSINO)
		    Lf->fs_ino = (unsigned long)mp->inode;
#endif	/* defined(HASFSINO) */

		    break;
		}
	    }
	} else
	    mp = (struct mounts *)NULL;
/*
 * If no offset has been activated and no size saved, activate the offset or
 * save the size.
 */
	if (!Lf->off_def && !Lf->sz_def) {
	    if (Foffset)
		Lf->off_def = 1;
	    else {
		switch (Ntype) {
		case N_CHR:
		case N_FIFO:
		    Lf->off_def = 1;
		    break;
		default:
		    Lf->sz = (SZOFFTYPE)pd->psfd_size;
		    Lf->sz_def = 1;
		}
	    }
	}
/*
 * See if this is an NFS file.
 */
	if (Fnfs) {
	    if (HasNFS < 0)
		(void) scanmnttab();
	    if (HasNFS && mp && mp->is_nfs)
		Lf->sf |= SELNFS;
	}
/*
 * Test for specified file.
 */
	if (Sfile && is_file_named(NULL,
				   ((Ntype == N_CHR) || (Ntype == N_BLK) ? 1
									 : 0)))
	    Lf->sf |= SELNM;
/*
 * Enter name characters.
 */
	if (!Lf->nm && Namech[0])
	    enter_nm(Namech);
}
