/*
 * dnode1.c - NetBSD and OpenBSD node functions for lsof
 *
 * This module must be separate to keep separate the multiple kernel inode
 * structure definitions.
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
"@(#) Copyright 1994 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dnode1.c,v 1.8 2005/08/08 19:53:24 abe Exp $";
#endif


#include "lsof.h"

#if	defined(HAS9660FS)
/*
 * Undo some conflicting node header file definitions.
 */

#undef	doff_t
#undef	i_dev
#undef	i_devvp
#undef	i_number
#undef	IN_ACCESS
#undef	IN_LOCKED
#undef	i_size
#undef	IN_WANTED


/*
 * At last, #include the desired header files.
 */

# if	HAS9660FS==1
#include <isofs/cd9660/iso.h>
#include <isofs/cd9660/cd9660_node.h>
# else	/* HAS9660FS!=1 */
#include <fs/cd9660/iso.h>
#include <fs/cd9660/cd9660_node.h>
# endif	/* HAS9660FS==1 */


/*
 * read_iso_node() -- read CD 9660 iso_node
 */

int
read_iso_node(v, d, ino, nl, sz)
	struct vnode *v;		/* containing vnode */
	dev_t *d;			/* returned device number */
	INODETYPE *ino;			/* returned inode number */
	long *nl;			/* returned link count */
	SZOFFTYPE *sz;			/* returned size */
{
	struct iso_node i;

	if (!v->v_data
	||  kread((KA_T)v->v_data, (char *)&i, sizeof(i)))
	    return(1);
	*d = i.i_dev;
	*ino = (INODETYPE)i.i_number;
	*nl = i.inode.iso_links;
	*sz = (SZOFFTYPE)i.i_size;
	return(0);
}
#endif	/* defined(HAS9660FS) */
