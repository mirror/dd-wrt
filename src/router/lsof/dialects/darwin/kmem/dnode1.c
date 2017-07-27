/*
 * dnode1.c - Darwin node functions for /dev/kmem-based lsof
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
static char *rcsid = "$Id: dnode1.c,v 1.3 2005/11/01 20:24:51 abe Exp $";
#endif

#include "lsof.h"

#if	defined(HAS9660FS)

/*
 * Do a little preparation for #include'ing cd9660_node.h, then #include it.
 */

#undef	i_size;
#undef	doff_t
#undef	IN_ACCESS

struct vop_abortop_args	 { int dummy; };
struct vop_access_args	 { int dummy; };
struct vop_blkatoff_args { int dummy; };
struct vop_bmap_args	 { int dummy; };
struct vop_close_args	 { int dummy; };
struct vop_getattr_args	 { int dummy; };
struct vop_inactive_args { int dummy; };
struct vop_ioctl_args	 { int dummy; };
struct vop_islocked_args { int dummy; };
struct vop_lock_args	 { int dummy; };
struct vop_lookup_args	 { int dummy; };
struct vop_mmap_args	 { int dummy; };
struct vop_open_args	 { int dummy; };
struct vop_pathconf_args { int dummy; };
struct vop_print_args	 { int dummy; };
struct vop_read_args	 { int dummy; };
struct vop_readdir_args	 { int dummy; };
struct vop_readlink_args { int dummy; };
struct vop_reclaim_args	 { int dummy; };
struct vop_seek_args	 { int dummy; };
struct vop_select_args	 { int dummy; };
struct vop_strategy_args { int dummy; };
struct vop_unlock_args	 { int dummy; };

#include <isofs/cd9660/cd9660_node.h>

/*
 * read_iso_node() -- read CD 9660 iso_node
 */

int
read_iso_node(v, d, dd, ino, nl, sz)
	struct vnode *v;		/* containing vnode */
	dev_t *d;			/* returned device number */
	int *dd;			/* returned device-defined flag */
	INODETYPE *ino;			/* returned inode number */
	long *nl;			/* returned number of links */
	SZOFFTYPE *sz;			/* returned size */
{

	struct iso_node i;

	if (!v->v_data
	||  kread((KA_T)v->v_data, (char *)&i, sizeof(i)))
	    return(1);

	*d = i.i_dev;
	*dd = 1;
	*ino = (INODETYPE)i.i_number;
	*nl = (long)i.inode.iso_links;
	*sz = (SZOFFTYPE)i.i_size;

	return(0);
}
#endif	/* defined(HAS9660FS) */
