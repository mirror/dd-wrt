/*
 * dnode.c - Linux node functions for /proc-based lsof
 */


/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1997 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dnode.c,v 1.25 2015/07/07 19:46:33 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local definitions
 */

#define	OFFSET_MAX	((off_t)0x7fffffff)	/* this is defined in
						 * .../src/fs/locks.c and not
						 * in a header file */
#define	PIDBUCKS	64			/* PID hash buckets */
#define	PINFOBUCKS	512			/* pipe info hash buckets */
#define	HASHPID(pid)	(((int)((pid * 31415) >> 3)) & (PIDBUCKS - 1))
#define	HASHPINFO(ino)	(((int)((ino * 31415) >> 3)) & (PINFOBUCKS - 1))


/*
 * Local structure definitions
 */

struct llock {
	int pid;
	dev_t dev;
	INODETYPE inode;
	char type;
	struct llock *next;
};


/*
 * Local definitions
 */

struct llock **LckH = (struct llock **)NULL; /* PID-hashed locks */


/*
 * Local function prototypes
 */

_PROTOTYPE(static void check_lock,(void));

#if	defined(HASEPTOPTS)
_PROTOTYPE(static void enter_pinfo,(void));
#endif	/* defined(HASEPTOPTS) */


/*
 * Local storage
 */

#if	defined(HASEPTOPTS)
static pxinfo_t **Pinfo = (pxinfo_t **)NULL;
#endif	/* defined(HASEPTOPTS) */


/*
 * check_lock() - check lock for file *Lf, process *Lp
 */

static void
check_lock()
{
	int h;
	struct llock *lp;

	h = HASHPID(Lp->pid);
	for (lp = LckH[h]; lp; lp = lp->next) {
	    if (Lp->pid == lp->pid
	    &&  Lf->dev == lp->dev
	    &&  Lf->inode == lp->inode)
	    {
		Lf->lock = lp->type;
		return;
	    }
	}
}


#if	defined(HASEPTOPTS)
/*
 * clear_pinfo() -- clear allocated pipe info
 */

void
clear_pinfo()
{
	int h;				/* hash index */
	pxinfo_t *pi, *pp;		/* temporary pointers */

	if (!Pinfo)
	    return;
	for (h = 0; h < PINFOBUCKS; h++) {
	    if ((pi = Pinfo[h])) {
		do {
		    pp = pi->next;
		    (void) free((FREE_P *)pi);
		    pi = pp;
		} while (pi);
		Pinfo[h] = (pxinfo_t *)NULL;
	    }
	}
}


/*
 * enter_pinfo() -- enter pipe info
 *
 * 	entry	Lf = local file structure pointer
 * 		Lp = local process structure pointer
 */

static void
enter_pinfo()
{
	int h;				/* hash result */
	struct lfile *lf;		/* local file structure pointer */
	struct lproc *lp;		/* local proc structure pointer */
	pxinfo_t *np, *pi, *pe;		/* pipe info pointers */

	if (!Pinfo) {
	/*
	 * Allocate pipe info hash buckets.
	 */
	    if (!(Pinfo = (pxinfo_t **)calloc(PINFOBUCKS, sizeof(pxinfo_t *))))
	    {
		(void) fprintf(stderr,
		    "%s: no space for %d pipe info buckets\n", Pn, PINFOBUCKS);
		    Exit(1);
	    }
	}
    /*
     * Make sure this is a unique entry.
     */
	for (h = HASHPINFO(Lf->inode), pi = Pinfo[h], pe = (pxinfo_t *)NULL;
	     pi;
	     pe = pi, pi = pi->next
	) {
	    lf = pi->lf;
	    lp = &Lproc[pi->lpx];
	    if (pi->ino == Lf->inode) {
		if ((lp->pid == Lp->pid) && !strcmp(lf->fd, Lf->fd))
		    return;
	    }
	}
   /*
    * Allocate, fill and link a new pipe info structure to the end of
    * the pipe inode hash chain.
    */
	if (!(np = (pxinfo_t *)malloc(sizeof(pxinfo_t)))) {
	    (void) fprintf(stderr,
		"%s: no space for pipeinfo, PID %d, FD %s\n",
		Pn, Lp->pid, Lf->fd);
	    Exit(1);
	}
	np->ino = Lf->inode;
	np->lf = Lf;
	np->lpx = Lp - Lproc;
	np->next = (pxinfo_t *)NULL;
	if (pe)
	    pe->next = np;
	else
	    Pinfo[h] = np;
}


/*
 * find_pendinfo() -- find pipe end info
 */

pxinfo_t *
find_pendinfo(lf, pp)
	struct lfile *lf;		/* pipe's lfile */
	pxinfo_t *pp;			/* previous pipe info (NULL == none) */
{
	struct lfile *ef;		/* pipe end local file structure */
	int h;				/* hash result */
	pxinfo_t *pi;			/* pipe info pointer */

	if (Pinfo) {
	    if (pp)
		pi = pp;
	     else {
		h = HASHPINFO(lf->inode);
		pi = Pinfo[h];
	    }
	    while (pi) {
		if (pi->ino == lf->inode) {
		    ef = pi->lf;
		    if (strcmp(lf->fd, ef->fd))
			return(pi);
	 	}
		pi = pi->next;
	    }
	}
	return((pxinfo_t *)NULL);

}
#endif	/* defined(HASEPTOPTS) */



/*
 * get_fields() - separate a line into fields
 */

int
get_fields(ln, sep, fr, eb, en)
	char *ln;			/* input line */
	char *sep;			/* separator list */
	char ***fr;			/* field pointer return address */
	int *eb;			/* indexes of fields where blank or an
					 * entry from the separator list may be
					 * embedded and are not separators
					 * (may be NULL) */
	int en;				/* number of entries in eb[] (may be
					 * zero) */
{
	char *bp, *cp, *sp;
	int i, j, n;
	MALLOC_S len;
	static char **fp = (char **)NULL;
	static int nfpa = 0;

	for (cp = ln, n = 0; cp && *cp;) {
	    for (bp = cp; *bp && (*bp == ' ' || *bp == '\t'); bp++);
		;
	    if (!*bp || *bp == '\n')
		break;
	    for (cp = bp; *cp; cp++) {
		if (*cp == '\n') {
		    *cp = '\0';
		    break;
		}
		if (*cp == '\t')	/* TAB is always a separator */
		    break;
		if (*cp == ' ')  {

		/*
		 * See if this field may have an embedded space.
		 */
		    if (!eb || !en)
			break;
		    else {
			for (i = j = 0; i < en; i++) {
			    if (eb[i] == n) {
				j = 1;
				break;
			    }
			}
			if (!j)
			    break;
		    }
		}
		if (sep) {

		/*
		 * See if the character is in the separator list.
		 */
		    for (sp = sep; *sp; sp++) {
			if (*sp == *cp)
			    break;
		    }
		    if (*sp) {

		    /*
		     * See if this field may have an embedded separator.
		     */
			if (!eb || !en)
			    break;
			else {
			    for (i = j = 0; i < en; i++) {
				if (eb[i] == n) {
				    j = 1;
				    break;
				}
			    }
			    if (!j)
				break;
			}
		    }
		}
	    }
	    if (*cp)
		*cp++ = '\0';
	    if (n >= nfpa) {
		nfpa += 32;
		len = (MALLOC_S)(nfpa * sizeof(char *));
		if (fp)
		    fp = (char **)realloc((MALLOC_P *)fp, len);
		else
		    fp = (char **)malloc(len);
		if (!fp) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for field pointers.\n",
			Pn, (int)len);
		    Exit(1);
		}
	    }
	    fp[n++] = bp;
	}
	*fr = fp;
	return(n);
}


/*
 * get_locks() - get lock information from /proc/locks
 */

void
get_locks(p)
	char *p;				/* /proc lock path */
{
	unsigned long bp, ep;
	char buf[MAXPATHLEN], *ec, **fp;
	dev_t dev;
	int ex, i, h, mode, pid;
	INODETYPE inode;
	struct llock *lp, *np;
	FILE *ls;
	long maj, min;
	char type;
	static char *vbuf = (char *)NULL;
	static size_t vsz = (size_t)0;
/*
 * Destroy previous lock information.
 */
	if (LckH) {
	    for (i = 0; i < PIDBUCKS; i++) {
		for (lp = LckH[i]; lp; lp = np) {
		    np = lp->next;
		    (void) free((FREE_P *)lp);
		}
		LckH[i] = (struct llock *)NULL;
	    }
	} else {

	/*
	 * If first time, allocate the lock PID hash buckets.
	 */
	    LckH = (struct llock **)calloc((MALLOC_S)PIDBUCKS,
					   sizeof(struct llock *));
	    if (!LckH) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d lock hash bytes\n",
		    Pn, (int)(sizeof(struct llock *) * PIDBUCKS));
		Exit(1);
	    }
	}
/*
 * Open the /proc lock file, assign a page size buffer to its stream,
 * and read it.
 */
	if (!(ls = open_proc_stream(p, "r", &vbuf, &vsz, 0)))
	    return;
	while (fgets(buf, sizeof(buf), ls)) {
	    if (get_fields(buf, ":", &fp, (int *)NULL, 0) < 10)
		continue;
	    if (!fp[1] || strcmp(fp[1], "->") == 0)
		continue;
	/*
	 * Get lock type.
	 */
	    if (!fp[3])
		continue;
	    if (*fp[3] == 'R')
		mode = 0;
	    else if (*fp[3] == 'W')
		mode = 1;
	    else
		continue;
	/*
	 * Get PID.
	 */
	    if (!fp[4] || !*fp[4])
		continue;
	    pid = atoi(fp[4]);
	/*
	 * Get device number.
	 */
	    ec = (char *)NULL;
	    if (!fp[5] || !*fp[5]
	    ||  (maj = strtol(fp[5], &ec, 16)) == LONG_MIN || maj == LONG_MAX
	    ||  !ec || *ec)
		continue;
	    ec = (char *)NULL;
	    if (!fp[6] || !*fp[6]
	    ||  (min = strtol(fp[6], &ec, 16)) == LONG_MIN || min == LONG_MAX
	    ||  !ec || *ec)
		continue;
	    dev = (dev_t)makedev((int)maj, (int)min);
	/*
	 * Get inode number.
	 */
	    ec = (char *)NULL;
	    if (!fp[7] || !*fp[7]
	    ||  (inode = strtoull(fp[7], &ec, 0)) == ULONG_MAX
	    ||  !ec || *ec)
		continue;
	/*
	 * Get lock extent.  Convert it and the lock type to a lock character.
	 */
	    if (!fp[8] || !*fp[8] || !fp[9] || !*fp[9])
		continue;
	    ec = (char *)NULL;
	    if ((bp = strtoul(fp[8], &ec, 0)) == ULONG_MAX || !ec || *ec)
		continue;
	    if (!strcmp(fp[9], "EOF"))		/* for Linux 2.4.x */
		ep = OFFSET_MAX;
	    else {
		ec = (char *)NULL;
		if ((ep = strtoul(fp[9], &ec, 0)) == ULONG_MAX || !ec || *ec)
		    continue;
	    }
	    ex = ((off_t)bp == (off_t)0 && (off_t)ep == OFFSET_MAX) ? 1 : 0;
	    if (mode)
		type = ex ? 'W' : 'w';
	    else
		type = ex ? 'R' : 'r';
	/*
	 * Look for this lock via the hash buckets.
	 */
	    h = HASHPID(pid);
	    for (lp = LckH[h]; lp; lp = lp->next) {
		if (lp->pid == pid
		&&  lp->dev == dev
		&&  lp->inode == inode
		&&  lp->type == type)
		    break;
	    }
	    if (lp)
		continue;
	/*
	 * Allocate a new llock structure and link it to the PID hash bucket.
	 */
	    if (!(lp = (struct llock *)malloc(sizeof(struct llock)))) {
		(void) snpf(buf, sizeof(buf), InodeFmt_d, inode);
		(void) fprintf(stderr,
		    "%s: can't allocate llock: PID %d; dev %x; inode %s\n",
		    Pn, pid, (int)dev, buf);
		Exit(1);
	    }
	    lp->pid = pid;
	    lp->dev = dev;
	    lp->inode = inode;
	    lp->type = type;
	    lp->next = LckH[h];
	    LckH[h] = lp;
	}
	(void) fclose(ls);
}


/*
 * process_proc_node() - process file node
 */

void
process_proc_node(p, pbr, s, ss, l, ls)
	char *p;			/* node's readlink() path */
	char *pbr;			/* node's path before readlink() */
	struct stat *s;			/* stat() result for path */
	int ss;				/* *s status -- i.e., SB_* values */
	struct stat *l;			/* lstat() result for FD (NULL for
					 * others) */
	int ls;				/* *l status -- i.e., SB_* values */
{
	mode_t access;
	mode_t type = 0;
	char *cp;
	struct mounts *mp = (struct mounts *)NULL;
	size_t sz;
	char *tn;
/*
 * Set the access mode, if possible.
 */
	if (l && (ls & SB_MODE) && ((l->st_mode & S_IFMT) == S_IFLNK)) {
	    if ((access = l->st_mode & (S_IRUSR | S_IWUSR)) == S_IRUSR)
		Lf->access = 'r';
	    else if (access == S_IWUSR)
		Lf->access = 'w';
	    else
		Lf->access = 'u';
	}
/*
 * Determine node type.
 */
	if (ss & SB_MODE) {
	    type = s->st_mode & S_IFMT;
	    switch (type) {
	    case S_IFBLK:
		Lf->ntype = Ntype = N_BLK;
		break;
	    case S_IFCHR:
		Lf->ntype = Ntype = N_CHR;
		break;
	    case S_IFIFO:
		Lf->ntype = Ntype = N_FIFO;
		break;
	    case S_IFSOCK:
		/* Lf->ntype = Ntype = N_REGLR;		by alloc_lfile() */
		process_proc_sock(p, pbr, s, ss, l, ls);
		return;
	    case 0:
		if (!strcmp(p, "anon_inode"))
		   Lf->ntype = Ntype = N_ANON_INODE;
		break;
	    }
	}
	if (Selinet)
	    return;
/*
 * Save the device.  If it is an NFS device, change the node type to N_NFS.
 */
	if (ss & SB_DEV) {
	    Lf->dev = s->st_dev;
	    Lf->dev_def = 1;
	}
	if ((Ntype == N_CHR || Ntype == N_BLK)) {
	    if (ss & SB_RDEV) {
		Lf->rdev = s->st_rdev;
		Lf->rdev_def = 1;
	    }
	}
	if (Ntype == N_REGLR && (HasNFS == 2)) {
	    for (mp = readmnt(); mp; mp = mp->next) {
		if ((mp->ty == N_NFS)
		&&  (mp->ds & SB_DEV) && Lf->dev_def && (Lf->dev == mp->dev)
		&&  (mp->dir && mp->dirl
		&&   !strncmp(mp->dir, p, mp->dirl))
		) {
		    Lf->ntype = Ntype = N_NFS;
		    break;
		}
	    }
	}
/*
 * Save the inode number.
 */
	if (ss & SB_INO) {
	    Lf->inode = (INODETYPE)s->st_ino;
	    Lf->inp_ty = 1;

#if	defined(HASEPTOPTS)
	    if ((Lf->ntype == N_FIFO) && FeptE) {
	    	(void) enter_pinfo();
		Lf->sf |= SELPINFO;
	    }
#endif	/* defined(HASEPTOPTS) */

	}
/*
 * Check for a lock.
 */
	if (Lf->dev_def && (Lf->inp_ty == 1))
	    (void) check_lock();
/*
 * Save the file size.
 */
	switch (Ntype) {
	case N_BLK:
	case N_CHR:
	case N_FIFO:
	    if (!Fsize && l && (ls & SB_SIZE) && OffType) {
		Lf->off = (SZOFFTYPE)l->st_size;
		Lf->off_def = 1;
	    }
	    break;
	default:
	    if (Foffset) {
		if (l && (ls & SB_SIZE) && OffType) {
		    Lf->off = (SZOFFTYPE)l->st_size;
		    Lf->off_def = 1;
		}
	    } else if (!Foffset || Fsize) {
		if (ss & SB_SIZE) {
		    Lf->sz = (SZOFFTYPE)s->st_size;
		    Lf->sz_def = 1;
		}
	    }
	}
/*
 * Record the link count.
 */
	if (Fnlink && (ss & SB_NLINK)) {
	    Lf->nlink = (long)s->st_nlink;
	    Lf->nlink_def = 1;
	    if (Nlink && (Lf->nlink < Nlink))
		Lf->sf |= SELNLINK;
	}
/*
 * Format the type name.
 */
	if (ss & SB_MODE) {
	    switch (type) {
	    case S_IFBLK:
		tn = "BLK";
		break;
	    case S_IFCHR:
		tn = "CHR";
		break;
	    case S_IFDIR:
		tn = "DIR";
		break;
	    case S_IFIFO:
		tn = "FIFO";
		break;
	    case S_IFREG:
		tn = "REG";
		break;
	    case S_IFLNK:
		tn = "LINK";
		break;
	    case S_ISVTX:
		tn = "VTXT";
		break;
	    default:
		if (Ntype == N_ANON_INODE)
		    tn = "a_inode";
		else {
		    (void) snpf(Lf->type, sizeof(Lf->type), "%04o",
			((type >> 12) & 0xf));
		    tn = (char *)NULL;
		}
	    }
	} else
	    tn = "unknown";
	if (tn)
	    (void) snpf(Lf->type, sizeof(Lf->type), "%s", tn);
/*
 * Record an NFS file selection.
 */
	if (Ntype == N_NFS && Fnfs)
	    Lf->sf |= SELNFS;
/*
 * Test for specified file.
 */
	if (Sfile
	&& is_file_named(1, p, mp,
			 ((type == S_IFCHR) || (type == S_IFBLK)) ? 1 : 0))
	    Lf->sf |= SELNM;
/*
 * If no NAME information has been stored, store the path.
 *
 * Store the remote host and mount point for an NFS file.
 */
	if (!Namech[0]) {
	    (void) snpf(Namech, Namechl, "%s", p);
	    if ((Ntype == N_NFS) && mp && mp->fsname) {
		cp = endnm(&sz);
		(void) snpf(cp, sz, " (%s)", mp->fsname);
	    }
	}
	if (Namech[0])
	    enter_nm(Namech);
}
