/*
 * dfile.c - /dev/kmem-based HP-UX file processing functions for lsof
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
static char *rcsid = "$Id: dfile.c,v 1.14 2001/08/14 13:27:16 abe Exp $";
#endif

#if	defined(HPUXKERNBITS) && HPUXKERNBITS>=64
#define _TIME_T
typedef int time_t;
/*
 * CAUTION!!! CAUTION!!! CAUTION!!! CAUTION!!! CAUTION!!! CAUTION!!!
 *
 * Do NOT:
 *
 *	#define INO_T
 *	typedef int ino_t;
 *
 * in this source file for HP-UX >= 10.30.  Doing so will cause the kernel's
 * ino_t type to be erroneously used instead of the application's.
 *
 * CAUTION!!! CAUTION!!! CAUTION!!! CAUTION!!! CAUTION!!! CAUTION!!!
 */
#endif	/* defined(HPUXKERNBITS) && HPUXKERNBITS>=64 */

#include "lsof.h"


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


/*
 * print_dev() - print device
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
 * process_file() - process file
 */

void
process_file(fp)
	KA_T fp;			/* kernel file structure address */
{
	struct file f;
	int flag;

	if (kread((KA_T)fp, (char *)&f, sizeof(f))) {
	    (void) snpf(Namech, Namechl, "can't read file struct from %s",
		print_kptr(fp, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
	Lf->off = (SZOFFTYPE)f.f_offset;

	if (f.f_count) {

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
		Lf->fna = (KA_T)f.f_data;
		Lf->fsv |= FSV_NI;
	    }
#endif	/* defined(HASFSTRUCT) */

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
	 * Process structure by its type.
	 */
	    switch (f.f_type) {

#if	defined(DTYPE_LLA)
	    case DTYPE_LLA:
		process_lla((KA_T)f.f_data);
		return;
#endif	/* DTYPE_LLA */

	    case DTYPE_VNODE:
		process_node((KA_T)f.f_data);
		return;
	    case DTYPE_SOCKET:
		process_socket((KA_T)f.f_data);
		return;
	    default:
		if (!f.f_type || (f.f_ops && (KA_T)f.f_ops != Vnfops)) {
		    (void) snpf(Namech, Namechl,
			"%s file struct, ty=%#x, op=%#x",
			print_kptr(fp, (char *)NULL, 0), f.f_type, f.f_ops);
		    enter_nm(Namech);
		    return;
		}
	    }
	}
	enter_nm("no more information");
}


#if	HPUXV>=1030
/*
 * read_mi() - read stream's module information
 *
 * Note: this function is included in this module, because ino_t is not
 *	 redfined to the kernel's type, but is left at the application's type.
 *	 See the CAUTION statement inside the HPUXKERNBITS>=64 #if/#endif
 *	 block at the beginning of this file.
 */

int
read_mi(sh, ip, pcb, pn)
	KA_T sh;			/* stream head address */
	KA_T *ip;			/* returned IP q_ptr */
	KA_T *pcb;			/* returned TCP or UDP q_ptr */
	char **pn;			/* returned protocol name */
{
	struct l_dev *dp;
	char *ep = Namech;
	struct sth_s hd;
	int i;
	size_t len, ml;
	char mn[32];
	KA_T ka, qa;
	struct module_info mi;
	struct queue q;
	struct qinit qi;
	size_t sz = Namechl;

	if (!sh
	||  kread(sh, (char *)&hd, sizeof(hd))) {
	    (void) snpf(Namech, Namechl, "can't read stream head: %s",
		print_kptr(sh, (char *)NULL, 0));
	    return(1);
	}
	if (!Lf->rdev_def)
	    dp = (struct l_dev *)NULL;
	else
	    dp = lkupdev(&DevDev, &Lf->rdev, 1, 0);
	if (dp)
	    (void) snpf(ep, sz, "%s", dp->name);
	else
	    *ep = '\0';
/*
 * Follow the stream head to each of its queue structures, retrieving the
 * module names for each queue's q_info->qi_minfo->mi_idname chain of
 * structures.  Separate each additional name from the previous one with
 * "->".
 *
 * Ignore failures to read all but queue structure chain entries.
 *
 * Ignore module names that end in "head".
 *
 * Save the q_ptr value for "tcp" and "udp" modules.
 */
	ml = sizeof(mn) - 1;
	mn[ml] = '\0';
	*ip = *pcb = (KA_T)NULL;
	qa = (KA_T)hd.sth_wq;
	for (i = 0; i < 20; i++, qa = (KA_T)q.q_next) {
	    if (!qa || kread(qa, (char *)&q, sizeof(q)))
		break;
	    if (!(ka = (KA_T)q.q_qinfo) || kread(ka, (char *)&qi, sizeof(qi)))
		continue;
	    if (!(ka = (KA_T)qi.qi_minfo) || kread(ka, (char *)&mi, sizeof(mi)))
		continue;
	    if (!(ka = (KA_T)mi.mi_idname) || kread(ka, mn, ml))
		continue;
	    if ((len = strlen(mn)) < 1)
		continue;
	    if (len >= 3 && !strcmp(&mn[len - 3], "sth"))
		continue;
	    ep = endnm(&sz);
	    (void) snpf(ep, sz, "%s%s", (ep == Namech) ? "" : "->", mn);
	    if (!q.q_ptr)
		continue;
	    if (!*ip && !strcmp(mn, "ip")) {
		*ip = (KA_T)q.q_ptr;
		continue;
	    }
	    if (!*pcb && !strcmp(mn, "tcpm")) {
		*pcb = (KA_T)q.q_ptr;
		*pn = "TCP";
		continue;
	    }
	    if (!*pcb && !strcmp(mn, "udpm")) {
		*pcb = (KA_T)q.q_ptr;
		*pn = "UDP";
	    }
	}
	return(0);
}
#endif	/* HPUXV>=1030 */
