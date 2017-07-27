/*
 * dfile.c - SCO OpenServer file processing functions for lsof
 */


/*
 * Copyright 1995 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1995 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dfile.c,v 1.11 2000/12/04 14:32:49 abe Exp abe $";
#endif

#include "lsof.h"


/*
 * get_max_fd() - get maximum file descriptor plus one
 */

int
get_max_fd()
{

#if	defined(F_GETHFDO) || defined(_SC_OPEN_MAX)
	int nd;
#endif	/* defined(F_GETHFDO) || defined(_SC_OPEN_MAX) */

#if	defined(F_GETHFDO)
	if ((nd = fcntl(-1, F_GETHFDO, 0)) >= 0)
	    return(nd);
#endif	/* defined(F_GETHFDO) */

#if	defined(_SC_OPEN_MAX)
	if ((nd = sysconf(_SC_OPEN_MAX)) >= 0)
	    return(nd);
#endif	/* defined(_SC_OPEN_MAX) */

	return(getdtablesize());
}


/*
 * print_dev() - print dev
 */

char *
print_dev(lf, dev)
	struct lfile *lf;		/* file whose device is to be printed */
	dev_t *dev;			/* device to be printed */
{
	static char buf[128];

	(void) snpf(buf, sizeof(buf), "%d,%d",
	    lf->is_nfs ? ((~(*dev >> 8)) & 0xff) : emajor(*dev),
	    eminor(*dev));
	return(buf);
}


/*
 * print_ino() - print inode
 */

char *
print_ino(lf)
	struct lfile *lf;		/* file whose device is to be printed */
{
	static char buf[128];

	(void) snpf(buf, sizeof(buf), (lf->inode & 0x80000000) ? "%#x" : "%lu",
	    lf->inode);
	return(buf);
}


/*
 * process_file() - process file
 */

void
process_file(fp)
	KA_T fp;		/* kernel file structure address */
{
	struct file f;
	int flag;

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
	/*
	 * Process structure.
	 */

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
		Lf->fna = (KA_T)f.f_inode;
		Lf->fsv |= FSV_NI;
	    }
#endif	/* defined(HASFSTRUCT) */

	    process_node((KA_T)f.f_inode);
	    return;
	}
	enter_nm("no more information");
}
