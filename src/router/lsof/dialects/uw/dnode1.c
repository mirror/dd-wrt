/*
 * dnode1.c - SCO UnixWare node functions for lsof
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
static char *rcsid = "$Id: dnode1.c,v 1.6 2005/08/13 16:21:41 abe Exp $";
#endif


#include "lsof.h"

#if	!defined(DYNAMIC_STACK_TRACE)
#define	DYNAMIC_STACK_TRACE		/* suppress C's objection to a zero
					 * length st_buffer[] element in the
					 * stack_trace struct, defined in
					 * <sys/percpu.h> */
					 
#endif	/* !defined(DYNAMIC_STACK_TRACE) */

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
#include <sys/fs/s5inode.h>


/*
 * reads5lino() - read s5 inode's local inode information
 */

int
reads5lino(v, i)
	struct vnode *v;		/* containing vnode */
	struct l_ino *i;		/* local inode information */
{
	struct inode s5i;

	if (kread((KA_T)v->v_data, (char *)&s5i, sizeof(s5i)))
	    return(1);
	i->dev = s5i.i_dev;
	i->dev_def = 1;
	i->rdev = s5i.i_rdev;
	i->rdev_def = 1;
	i->nlink = (long)s5i.i_nlink;
	i->nlink_def = 1;
	i->nm = (char *)NULL;
	i->number = (INODETYPE)s5i.i_number;
	i->number_def = 1;
	i->size = s5i.i_size;
	i->size_def = 1;
	return(0);
}
