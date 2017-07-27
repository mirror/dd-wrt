/*
 * dnode1.c - /dev/kmem-based HP-UX node functions for lsof
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
static char *rcsid = "$Id: dnode1.c,v 1.11 2005/08/08 19:50:23 abe Exp $";
#endif


#if	defined(HASVXFS)

# if	defined(HPUXKERNBITS) && HPUXKERNBITS>=64
#define _INO_T
typedef int ino_t;
#define _TIME_T
typedef int time_t;
# endif	/* defined(HPUXKERNBITS) && HPUXKERNBITS>=64 */

#include "lsof.h"


/*
 * HP-UX versions below 10.20:
 *
 *    The pool_id_t type does not seem to be defined in the header  files
 *    distributed by HP.  However, <sys/fs/vx_hpux.h> requires  it when
 *    _KERNEL is defined.  So we fake the pool_id_t definition.
 *
 *    <sys/fs/vx_hpux.h> also requires sv_sema_t.  It's defined in
 *    <sys/sem_alpha.h> when _KERNEL is defined, but some other header file has
 *    already included <sys/sem_alpha.h> with _KERNEL undefined.  So we fake the
 *    sv_sema_t definition.
 *
 * HP-UX version 10.20 and above:
 *
 *    The pool_id_t type is used by other header files for other purposes.
 *    Redefine it for VXFS.  Delete some other conflicting definitions.
 *    Don't #define _KERNEL.  Include a different set of VXFS header files.
 */


# if	HPUXV>=1020
#undef	te_offset
#undef	i_size
#undef	di_size
#define	pool_id_t	vx_pool_id_t

#  if	HPUXV>=1030
#define	ulong	vx_ulong		/* avoid <sys/stream.h> conflict */
#  endif	/* HPUXV>=1030 */

#include <sys/fs/vx_hpux.h>
#include <sys/fs/vx_port.h>
#include <sys/fs/vx_inode.h>

#  if	HPUXV>=1030
#undef	ulong
#  endif	/* HPUXV>=1030 */

# else	/* HPUXV<1020 */

#define	pool_id_t	caddr_t
#define	sv_sema_t	caddr_t
#define	_KERNEL
#include <sys/fs/vx_hpux.h>
#include <sys/fs/vx_inode.h>
#undef	_KERNEL
# endif	/* HPUXV>=1020 */


/*
 * read_vxnode() - read Veritas file system inode information
 */

int
read_vxnode(v, vfs, dev, devs, rdev, rdevs)
	struct vnode *v;		/* local containing vnode */
	struct l_vfs *vfs;		/* local vfs structure */
	dev_t *dev;			/* device number receiver */
	int *devs;			/* device status receiver */
	dev_t *rdev;			/* raw device number receiver */
	int *rdevs;			/* raw device status receiver */
{
	struct vx_inode i;

	if (!v->v_data || kread((KA_T)v->v_data, (char *)&i, sizeof(i)))
	    return(1);
/*
 * Return device numbers.
 */
	if (vfs && vfs->fsname)
	    *dev = vfs->dev;
	else
	    *dev = i.i_dev;
	*devs = 1;
	if ((v->v_type == VCHR) || (v->v_type == VBLK)) {
	    *rdev = v->v_rdev;
	    *rdevs = 1;
	}
/*
 * Record inode number.
 */
	Lf->inode = (INODETYPE)i.i_number;
	Lf->inp_ty = 1;
/*
 * Record size.
 */
	if (Foffset || ((v->v_type == VCHR || v->v_type == VBLK) && !Fsize))
	    Lf->off_def = 1;
	else {
	    Lf->sz = (SZOFFTYPE)i.i_size;
	    Lf->sz_def = 1;
	}
/*
 * Record link count.
 */
	if (Fnlink) {
	    Lf->nlink = (long)i.i_nlink;
	    Lf->nlink_def = 1;
	    if (Nlink && (Lf->nlink < Nlink))
		Lf->sf |= SELNLINK;
	}
	return(0);
}
#endif	/* defined(HASVXFS) */
