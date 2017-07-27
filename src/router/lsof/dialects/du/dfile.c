/*
 * dfile.c - DEC OSF/1, Digital UNIX, Tru64 UNIX file processing functions for
 *	     lsof
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
static char *rcsid = "$Id: dfile.c,v 1.12 2001/08/14 12:40:12 abe Exp $";
#endif


#include "lsof.h"


#if	defined(HASIPv6)
/*
 * gethostbyname2() -- an RFC2133-compatible get-host-by-name-two function
 *		       to get AF_INET and AF_INET6 addresses from host names,
 *		       using the RFC2553-compatible getipnodebyname() function
 */

extern struct hostent *
gethostbyname2(nm, prot)
	char *nm;			/* host name */
	int prot;			/* protocol -- AF_INET or AF_INET6 */
{
	int err;
	static struct hostent *hep = (struct hostent *)NULL;

	if (hep)
	    (void) freehostent(hep);
	hep = getipnodebyname(nm, prot, 0, &err);
	return(hep);
}
#endif	/* defined(HASIPv6) */


#if	defined(HASPRIVNMCACHE)
/*
 * print_advfs_path() - print an ADVFS file path
 *
 * return: 1 if path printed
 *
 * This code was provided by Dean Brock <brock@cs.unca.edu>.
 *
 * This function is called by the name HASPRIVNMCACHE from printname().
 */

int
print_advfs_path(lf)
	struct lfile *lf;		/* file whose name is to be printed */
{
	char buf[MAXPATHLEN+1];
	mlBfTagT t2pb;
/*
 * Print any non-NULL path returned by tag_to_path() for ADVFS files that
 * have sequence and inode numbers.
 */
	if (!lf->advfs_seq_stat || lf->inp_ty != 1 || !lf->fsdir || !*lf->fsdir)
	    return(0);
	t2pb.ml_ino = (int)lf->inode;
	t2pb.ml_seq = lf->advfs_seq;
	if (tag_to_path(lf->fsdir, t2pb, MAXPATHLEN, buf) || !*buf)
	    return(0);
	buf[MAXPATHLEN] = '\0';
	safestrprt((buf[0] == '/' && buf[1] == '/') ? &buf[1] : buf, stdout, 0);
	return(1);
}
#endif	/* defined(HASPRIVNMCACHE) */


/*
 * print_dev() - print device
 */

char *
print_dev(lf, dev)
	struct lfile *lf;		/* file whose device is to be printed */
	dev_t *dev;			/* device to be printed */
{
	static char buf[128];

	if (GET_MIN_DEV(*dev) > 9999999)
	    (void) snpf(buf, sizeof(buf), "%d,%#x", GET_MAJ_DEV(*dev),
			GET_MIN_DEV(*dev));
	else
	    (void) snpf(buf, sizeof(buf), "%d,%d", GET_MAJ_DEV(*dev),
			GET_MIN_DEV(*dev));
	return(buf);
}
