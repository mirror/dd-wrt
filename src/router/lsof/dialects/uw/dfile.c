/*
 * dfile.c - SCO UnixWare file processing functions for lsof
 */


/*
 * Copyright 1996 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1996 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dfile.c,v 1.11 2000/08/01 15:26:38 abe Exp $";
#endif


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
 * process_file() - process file
 */

void
process_file(fp)
	KA_T fp;			/* kernel file structure address */
{
	struct file f;
	int flag;

#if     defined(FILEPTR)
	FILEPTR = &f;
#endif  /* defined(FILEPTR) */

	if (kread(fp, (char *)&f, sizeof(f))) {
	    (void) snpf(Namech, Namechl, "can't read file struct from %s",
		print_kptr(fp, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
	Lf->off = (SZOFFTYPE)f.f_offset;

	if (f.f_count

#if	defined(HAS_F_OPEN)
	|| f.f_open
#endif	/* defined(HAS_F_OPEN) */

    	) {

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
	 * Process structure.
	 */
	    process_node((KA_T)f.f_vnode);
		return;
	}
	enter_nm("no more information");
}


/*
 * Case independent character mappings -- for strcasecmp() and strncasecmp()
 */

static short CIMap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
	'\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
	'\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
	'\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};


/*
 * strcasecmp() - case insentitive character compare (from BSD)
 */

int
strcasecmp(s1, s2)
	char *s1, *s2;
{
	short *mp = CIMap;
	unsigned char *cp1 = (unsigned char *)s1;
	unsigned char *cp2 = (unsigned char *)s2;

	while (mp[*cp1] == mp[*cp2]) {
	    if (*cp1++ == '\0')
		return(0);
	    cp2++;
	}
	return ((int)(mp[*cp1] - mp[*cp2]));
}


/*
 * strncasecmp() - case insensitive character compare, limited by length
 *		   (from BSD)
 */

int
strncasecmp(s1, s2, n)
	char *s1, *s2;
	int n;
{
	short *mp = CIMap;
	unsigned char *cp1, *cp2;

	if (n) {
	    cp1 = (unsigned char *)s1;
	    cp2 = (unsigned char *)s2;
	    do {
		if (mp[*cp1] != mp[*cp2])
		    return((int)(mp[*cp1] - mp[*cp2]));
		if (*cp1++ == '\0')
		    break;
		cp2++;
	    } while (--n != 0);
	}
	return(0);
}
