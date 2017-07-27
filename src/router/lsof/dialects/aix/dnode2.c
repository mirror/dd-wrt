/*
 * dnode2.c - AIX jfs2 support
 *
 * V. Abell
 * Purdue University
 */


/*
 * Copyright 2003 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 2003 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dnode2.c,v 1.4 2005/08/08 19:46:38 abe Exp $";
#endif


#if	defined(HAS_JFS2)
#define	_H_JFS_INO				/* prevent <jfs_ino.h> */
#define	_H_JFS_INODE				/* prevent <jfs_inode.h> */
#define	PROTO_H					/* prevent "proto.h" and
						 * "dproto.h" until struct
						 * inode is available from
						 * <j2/j2_inode.h> */
#define	DPROTO_H
#include "lsof.h"
#define	_KERNEL
#include <j2/j2_inode.h>
#undef	PROTO_H					/* enable "proto.h" */
#undef	DPROTO_H				/* enable "dproto.h" */
#include "proto.h"
#include "dproto.h"

int
readj2lino(ga, li)
	struct gnode *ga;			/* gnode address */
	struct l_ino *li;			/* local inode receiver */
{
	struct inode i;				/* jfs2 inode */
/*
 * Read the jfs2 inode and fill in the local inode receiver.
 *
 * Note: the caller is responsible for initializing *li to zeroes.
 */
	if (!ga
	||  !ga->gn_data
	||  kread((KA_T)ga->gn_data, (char *)&i, sizeof(i)))
	    return(1);
	li->dev = i.i_dev;
	li->nlink = i.i_nlink;
	li->number = (INODETYPE)i.i_number;
	li->size = i.i_size;
	li->dev_def = li->nlink_def = li->number_def = li->size_def = 1;
	return(0);
} 
#endif	/* defined(HAS_JFS2) */
