/*
 * dnode1.c - FreeBSD node functions for lsof
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
static char *rcsid = "$Id: dnode1.c,v 1.10 2008/10/21 16:16:06 abe Exp abe $";
#endif


#include "lsof.h"

#if	defined(HAS9660FS)

/*
 * Do a little preparation for #include'ing cd9660_node.h, then #include it.
 */

#undef	i_size
#undef	doff_t
#undef	IN_ACCESS

#  if	FREEBSDV>=4000 && defined(__alpha__)
#define	dev_t	void *
#  endif	/* FREEBSDV>=4000 && defined(__alpha__) */

#include "cd9660_node.h"

# if	defined(HAS_NO_ISO_DEV)
#define	_KERNEL
#include <isofs/cd9660/iso.h>
#undef	_KERNEL
# endif	/* defined(HAS_NO_ISO_DEV) */

#  if	FREEBSDV>=4000 && defined(__alpha__)
#undef	dev_t
#  endif	/* FREEBSDV>=4000 && defined(__alpha__) */


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

# if	FREEBSDV<2000
	struct iso_node *ip;
# else	/* FREEBSDV>=2000 */
	struct iso_node i;
# endif	/* FREEBSDV<2000 */

# if	FREEBSDV>=4000
#  if	FREEBSDV<5000
	struct specinfo udev;
#  else	/* FREEBSDV>=5000 */
	struct cdev udev;
#   if	defined(HAS_NO_ISO_DEV)
	struct iso_mnt im;
#   endif	/* defined(HAS_NO_ISO_DEV) */
#  endif	/* FREEBSDV<5000 */
# endif	/* FREEBSDV>=4000 */

# if	FREEBSDV<2000
	ip = (struct iso_node *)v->v_data;
	*d = ip->i_dev;
	*dd = 1;
	*ino = (INODETYPE)ip->i_number;
	*nl = (long)ip->inode.iso_links;
	*sz = (SZOFFTYPE)ip->i_size;
# else	/* FREEBSDV>=2000 */
	if (!v->v_data
	||  kread((KA_T)v->v_data, (char *)&i, sizeof(i)))
	    return(1);

# if	FREEBSDV>=4000
#  if	defined(HAS_NO_ISO_DEV)
	if (i.i_mnt && !kread((KA_T)i.i_mnt, (char *)&im, sizeof(im))
	&&  im.im_dev && !kread((KA_T)im.im_dev, (char *)&udev, sizeof(udev)))
#  else	/* !defined(HAS_NO_ISO_DEV) */
	if (i.i_dev && !kread((KA_T)i.i_dev, (char *)&udev, sizeof(udev)))
#  endif	/* defined(HAS_NO_ISO_DEV) */

	{

# if	defined(HAS_NO_SI_UDEV)
	    *d = Dev2Udev(&udev);
# else	/* !defined(HAS_NO_SI_UDEV) */
	    *d = udev.si_udev;
# endif	/* defined(HAS_NO_SI_UDEV) */

	    *dd = 1;
	}
# else	/* FREEBSDV<4000 */
	*d = i.i_dev;
	*dd = 1;
# endif	/* FREEBSDV>=4000 */

	*ino = (INODETYPE)i.i_number;
	*nl = (long)i.inode.iso_links;
	*sz = (SZOFFTYPE)i.i_size;
# endif	/* FREEBSDV<2000 */

	return(0);
}
#endif	/* defined(HAS9660FS) */
