/*
 * dnode2.c - SCO UnixWare node functions for lsof
 *
 * This module must be separate to keep separate the multiple kernel inode
 * structure definitions.
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
static char *rcsid = "$Id: dnode2.c,v 1.7 2005/08/13 16:21:41 abe Exp $";
#endif


#include "lsof.h"

#if	defined(HASVXFS)

# if	UNIXWAREV<70000
#undef	fs_bsize
#undef	IFMT
#undef	IFIFO
#undef	IFCHR
#undef	IFDIR
#undef	IFNAM
#undef	IFBLK
#undef	IFREG
#undef	IFLNK
#undef	ISUID
#undef	ISGID
#undef	ISVTX
#undef	IREAD
#undef	IWRITE
#undef	IEXEC
#include <sys/fs/vx_inode.h>
# else	/* UNIXWAREV>=70000 */
struct vx_inode{
	unsigned long d1[28];
	dev_t i_dev;
	unsigned long i_number;
	unsigned long d2[76];
	unsigned long i_nlink;
	unsigned long d3[2];
	unsigned long long i_size;
	unsigned long d4[8];
	dev_t i_rdev;
};
# endif	/* UNIXWAREV<70000 */
#endif	/* defined(HASVXFS) */


/*
 * readvxfslino() - read vxfs inode's local inode information
 */

int
readvxfslino(v, i)
	struct vnode *v;		/* containing vnode */
	struct l_ino *i;		/* local inode information */
{

#if	defined(HASVXFS)
	struct vx_inode vx;

	if (kread((KA_T)v->v_data, (char *)&vx, sizeof(vx)))
	    return(1);
	i->dev = vx.i_dev;
	i->dev_def = 1;
	i->nlink = (long)vx.i_nlink;
	i->nlink_def = 1;
	i->nm = (char *)NULL;
	i->number = (INODETYPE)vx.i_number;
	i->number_def = 1;
	if (v->v_type == VCHR) {
	    i->rdev = vx.i_rdev;
	    i->rdev_def = 1;
	} else {
	    i->rdev = (dev_t)0;
	    i->rdev_def = 0;
	}
	i->size = (SZOFFTYPE)vx.i_size;
	i->size_def = 1;
	return(0);
#else	/* !defined(HASVXFS) */
	return(1);
#endif	/* defined(HASVXFS) */

}
