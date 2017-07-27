/*
 * dfile.c - Solaris file processing functions for lsof
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
static char *rcsid = "$Id: dfile.c,v 1.21 2009/03/25 19:22:16 abe Exp $";
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

static struct hsfile *HbyCd =		/* hash by clone buckets */
	(struct hsfile *)NULL;
static int HbyCdCt = 0;			/* HbyCd entry count */
static struct hsfile *HbyFdi =		/* hash by file buckets */
	(struct hsfile *)NULL;
static int HbyFdiCt = 0;		/* HbyFdi entry count */
static struct hsfile *HbyFrd =		/* hash by file raw device buckets */
	(struct hsfile *)NULL;
static int HbyFrdCt = 0;		/* HbyFrd entry count */
static struct hsfile *HbyFsd =		/* hash by file system buckets */
	(struct hsfile *)NULL;
static int HbyFsdCt = 0;		/* HbyFsd entry count */
static struct hsfile *HbyNm =		/* hash by name buckets */
	(struct hsfile *)NULL;
static int HbyNmCt = 0;			/* HbyNm entry count */


/*
 * Local definitions
 */

#define	SFCDHASH	1024		/* Sfile hash by clone device */
#define	SFDIHASH	4094		/* Sfile hash by (device,inode) number
					 * pair bucket count (power of 2!) */
#define	SFFSHASH	128		/* Sfile hash by file system device
					 * number bucket count (power of 2!) */
#define SFHASHDEVINO(maj, min, ino, mod) ((int)(((int)((((int)(maj+1))*((int)((min+1))))+ino)*31415)&(mod-1)))
					/* hash for Sfile by major device,
					 * minor device, and inode, modulo m
					 * (m must be a power of 2) */
#define	SFNMHASH	4096		/* Sfile hash by name bucket count
					   (power of 2!) */
#define	SFRDHASH	1024		/* Sfile hash by raw device number
					 * bucket count (power of 2!) */
#define SFHASHRDEVI(maj, min, rmaj, rmin, ino, mod) ((int)(((int)((((int)(maj+1))*((int)((min+1))))+((int)(rmaj+1)*(int)(rmin+1))+ino)*31415)&(mod-1)))
					/* hash for Sfile by major device,
					 * minor device, major raw device,
					 * minor raw device, and inode, modulo
					 * mod (mod must be a power of 2) */


#if	solaris<20500
/*
 * get_max_fd() - get maximum file descriptor plus one
 */

int
get_max_fd()
{
	struct rlimit r;

	if (getrlimit(RLIMIT_NOFILE, &r))
	    return(-1);
	return(r.rlim_cur);
}
#endif	/* solaris<20500 */


/*
 * hashSfile() - hash Sfile entries for use in is_file_named() searches
 */

void
hashSfile()
{
	int cmaj, hvc, i;
	static int hs = 0;
	struct sfile *s;
	struct hsfile *sh, *sn;
/*
 * Do nothing if there are no file search arguments cached or if the
 * hashes have already been constructed.
 */
	if (!Sfile || hs)
	    return;
/*
 * Preset the clone major device for Solaris.
 */
	if (HaveCloneMaj) {
	    cmaj = CloneMaj;
	    hvc = 1;
	} else
	    hvc = 0;
/*
 * Allocate hash buckets by clone device, (device,inode), file system device,
 * and file name.
 */
	if (hvc) {
	    if (!(HbyCd = (struct hsfile *)calloc((MALLOC_S)SFCDHASH,
						  sizeof(struct hsfile))))
	    {
		(void) fprintf(stderr,
		    "%s: can't allocate space for %d clone hash buckets\n",
		    Pn, SFCDHASH);
		Exit(1);
	    }
	}
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
 * Scan the Sfile chain, building file, file system, and file name hash
 * bucket chains.
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
		    if (s->type
		    &&  ((s->mode == S_IFCHR) || (s->mode == S_IFBLK)))
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
		} else {
		    if (!hvc || (GET_MAJ_DEV(s->rdev) != cmaj))
			continue;
		    sh = &HbyCd[SFHASHDEVINO(0, GET_MIN_DEV(s->rdev), 0,
					     SFCDHASH)];
		    HbyCdCt++;
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
 * is_file_named() - is this file named?
 */

int
is_file_named(p, nt, vt, ps)
	char *p;			/* path name; NULL = search by device
					 * and inode (from *Lf) */
	int nt;				/* node type -- e.g., N_* */
	enum vtype vt;			/* vnode type */
	int ps;				/* print status: 0 = don't copy name
					 * to Namech */
{
	char *ep;
	int f = 0;
	struct sfile *s;
	struct hsfile *sh;
	size_t sz;
/*
 * Check for a path name match, as requested.
 */
	if (p && HbyNmCt) {
	    for (sh = &HbyNm[hashbyname(p, SFNMHASH)]; sh; sh = sh->next) {
		if ((s = sh->s) && strcmp(p, s->aname) == 0) {
		    f = 2;
		    break;
		}
	    }
	}
/*
 * Check for a Solaris clone file.
 */
	if (!f && HbyCdCt && nt == N_STREAM && Lf->dev_def && Lf->rdev_def
	&&  (Lf->dev == DevDev))
	{
	    for (sh = &HbyCd[SFHASHDEVINO(0, GET_MAJ_DEV(Lf->rdev), 0,
					  SFCDHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s) && (GET_MAJ_DEV(Lf->rdev)
				==  GET_MIN_DEV(s->rdev)))
		{
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
	    for (sh = &HbyFdi[SFHASHDEVINO(GET_MAJ_DEV(Lf->dev),
					   GET_MIN_DEV(Lf->dev),
					   Lf->inode,
					   SFDIHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s) && (Lf->dev == s->dev)
		&&  (Lf->inode == s->i)) {
		    f = 1;
		    break;
		}
	    }
	}
/*
 * Check for a file system match.
 */
	if (!f && HbyFsdCt && Lf->dev_def) {
	    for (sh = &HbyFsd[SFHASHDEVINO(GET_MAJ_DEV(Lf->dev),
					   GET_MIN_DEV(Lf->dev), 0, SFFSHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s) && Lf->dev == s->dev) {
		    f = 1;
		    break;
		}
	    }
	}
/*
 * Check for a character or block device match.
 */
	if (!f && HbyFrdCt
	&&  ((vt = VCHR) || (vt = VBLK))
	&&  Lf->dev_def && (Lf->dev == DevDev)
	&&  Lf->rdev_def
	&& (Lf->inp_ty == 1 || Lf->inp_ty == 3))
	{
	    for (sh = &HbyFrd[SFHASHRDEVI(GET_MAJ_DEV(Lf->dev),
					  GET_MIN_DEV(Lf->dev),
					  GET_MAJ_DEV(Lf->rdev),
					  GET_MIN_DEV(Lf->rdev),
					  Lf->inode, SFRDHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s) && (s->dev == Lf->dev)
		&&  (s->rdev == Lf->rdev) && (s->i == Lf->inode))
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
		if (ps)
		    (void) snpf(Namech, Namechl, "%s", p);
	    } else {
		if (ps && s->type) {

		/*
		 * If the search argument isn't a file system, propagate it
		 * to Namech[]; otherwise, let printname() compose the name.
		 */
		    (void) snpf(Namech, Namechl, "%s", s->name);
		    if (s->devnm) {
			ep = endnm(&sz);
			(void) snpf(ep, sz, " (%s)", s->devnm);
		    }
		}
	    }
	    s->f = 1;
	    return(1);
	}
	return(0);
}


#if	defined(HASPRINTDEV)
/*
 * print_dev() - print device
 */

char *
print_dev(lf, dev)
	struct lfile *lf;		/* file whose device is to be printed */
	dev_t *dev;			/* device to be printed */
{
	static char buf[128];
/*
 * Avoid the Solaris major() and minor() functions from makedev(3C) to get
 * printable major/minor numbers.
 *
 * We would like to use the L_MAXMAJ definition from <sys/sysmacros.h> all
 * the time, but it's not always correct in all versions of Solaris.
 */
	(void) snpf(buf, sizeof(buf), "%d,%d", (int)((*dev >> L_BITSMINOR) &

#if	solaris>=20501
	    L_MAXMAJ
#else	/* solaris<20501 */
	    0x3fff
#endif	/* solaris>=20501 */

	    ), (int)(*dev & L_MAXMIN));
	return(buf);
}
#endif	/* defined(HASPRINTDEV) */


#if	defined(HAS_V_PATH)

/*
 * Local definitions
 */

#define	VPRDLEN	((MAXPATHLEN + 7)/8)	/* v_path read length increment */


/*
 * print_v_path() - print path name from vnode's v_path pointer
 */

extern int
print_v_path(lf)
	struct lfile *lf;		/* local file structure */
{
	char buf[MAXPATHLEN+1];
	unsigned char del = 0;
	unsigned char aperr = 0;

# if	defined(HASMNTSTAT)
	struct stat sb;
# endif	/* defined(HASMNTSTAT) */

# if	defined(HASVXFS) && defined(HASVXFSRNL)
	if (lf->is_vxfs && (lf->inp_ty == 1) && lf->fsdir) {
	    if (print_vxfs_rnl_path(lf))
		return(1);
	}
# endif	/* defined(HASVXFS) && defined(HASVXFSRNL) */

	(void) read_v_path((KA_T)lf->V_path, buf, (size_t)sizeof(buf));
	if (buf[0]) {

# if	defined(HASMNTSTAT)
	    if (!lf->mnt_stat && lf->dev_def && (lf->inp_ty == 1)) {

	    /*
	     * No problem was detected in applying stat(2) to this mount point.
	     * If the device and inode for the file are known, it is probably
	     * safe and worthwhile to apply stat(2) to the v_path.
	     */
		if (!statsafely(buf, &sb)) {

		/*
		 * The stat(2) succeeded.  See if the device and inode match.
		 * If they both don't match, ignore the v_path.
		 */
		    if ((lf->dev != sb.st_dev)
		    ||  (lf->inode != (INODETYPE)sb.st_ino)
		    ) {
			return(0);
		    }
		} else {

		/*
		 * The stat(2) failed.
		 *
		 * If the error reply is ENOENT and the -X option hasn't been
		 * specified, ignore the v_path.
		 *
		 * If the error reply is ENOENT, the -X option has been
		 * specified and the file's link count is zero, report the
		 * v_path with the "(deleted)" notation.
		 *
		 * If the error reply is EACCES or EPERM, report the v_path,
		 * followed by "(?)", because lsof probably lacks permission
		 * to apply stat(2) to v_path.
		 */
		    switch (errno) {
		    case EACCES:
		    case EPERM:
			aperr = 1;
			break;
		    case ENOENT:

# if	defined(HASXOPT)
			if (Fxopt && lf->nlink_def && !lf->nlink) {
			    del = 1;
			    break;
			}
# endif	/* defined(HASXOPT) */

			return(0);
		    default:
			return(0);
		    }
		}
	    }
# endif	/* defined(HASMNTSTAT) */

	/*
	 * Print the v_path.
	 */
	    safestrprt(buf, stdout, 0);
	    if (del)
		safestrprt(" (deleted)", stdout, 0);
	    else if (aperr)
		safestrprt(" (?)", stdout, 0);
	    return(1);
	}
	return(0);
}


/*
 * read_v_path() - read path name from vnode's v_path pointer
 */

extern void
read_v_path(ka, rb, rbl)
	KA_T ka;			/* kernel path address */
	char *rb;			/* receiving buffer */
	size_t rbl;			/* receiving buffer length */
{
	char *ba;
	size_t rl, tl;

	*rb = '\0';
	if (!ka)
	    return;
	for (ba = rb, tl = 0;
	     tl < (rbl - 1);
	     ba += rl, ka += (KA_T)((char *)ka + rl), tl += rl
	) {

	/*
	 * Read v_path VPRDLEN bytes at a time until the local buffer is full
	 * or a NUL byte is reached.
	 */
	    if ((rl = rbl - 1 - tl) > VPRDLEN)
		rl = VPRDLEN;
	    else if (rl < 1) {
		*(rb + rbl - 1) = '\0';
		break;
	    }
	    if (!kread(ka, ba, rl)) {
		*(ba + rl) = '\0';
		if (strchr(ba, '\0') < (ba + rl))
		    break;
	    } else {

	    /*
	     * Can't read a full buffer load; try reducing the length one
	     * byte at a time until it reaches zero.  Stop here, since it
	     * has been established that no more bytes can be read.
	     */
		for (rl--; rl > 0; rl--) {
		    if (!kread(ka, ba, rl)) {
			*(ba + rl) = '\0';
			break;
		    }
		}
		if (rl <= 0)
		    *ba = '\0';
		break;
	    }
	}
}
#endif	/* defined(HAS_V_PATH) */


/*
 * process_file() - process file
 */

void
process_file(fp)
	KA_T fp;		/* kernel file structure address */
{
	struct file f;
	int flag;

#if	defined(FILEPTR)
	FILEPTR = &f;
#endif	/* defined(FILEPTR) */

	if (kread(fp, (char *)&f, sizeof(f))) {
	    (void) snpf(Namech, Namechl, "can't read file struct from %s",
		print_kptr(fp, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
	Lf->off = (SZOFFTYPE)f.f_offset;

	if (f.f_count) {

	/*
	 * Construct access code.
	 */
	    if ((flag = (f.f_flag & (FREAD | FWRITE))) == FREAD)
		Lf->access = 'r';
	    else if (flag == FWRITE)
		Lf->access = 'w';
	    else if (flag == (FREAD | FWRITE))
		Lf->access = 'u';

#if	defined(HASFSTRUCT)
	/*
	 * Save file structure values.
	 */
	    if (Fsv & FSV_CT) {
		Lf->fct = (long)f.f_count;
		Lf->fsv |= FSV_CT;
	    }
	    if (Fsv & FSV_FA) {
		Lf->fsa = fp;
		Lf->fsv |= FSV_FA;
	    }
	    if (Fsv & FSV_FG) {
		Lf->ffg = (long)f.f_flag;
		Lf->fsv |= FSV_FG;
	    }
	    if (Fsv & FSV_NI) {
		Lf->fna = (KA_T)f.f_vnode;
		Lf->fsv |= FSV_NI;
	    }
#endif	/* defined(HASFSTRUCT) */

	/*
	 * Solaris file structures contain a vnode pointer.  Process it.
	 */
	    process_node((KA_T)f.f_vnode);
	    return;
	}
	enter_nm("no more information"); }


#if	defined(HASIPv6)
/*
 * gethostbyname2() -- an RFC2133-compatible get-host-by-name-two function
 *                     to get AF_INET and AF_INET6 addresses from host names,
 *                     using the RFC2553-compatible getipnodebyname() function
 */

extern struct hostent *
gethostbyname2(nm, prot)
	const char *nm; 		/* host name */
	int prot;			/* protocol -- AF_INET or AF_INET6 */
{
	int err;
	static struct hostent *hep = (struct hostent *)NULL;

	if (hep)
	    (void) freehostent(hep);
	return((hep = getipnodebyname(nm, prot, 0, &err)));
}
#endif	/* defined(HASIPv6) */
