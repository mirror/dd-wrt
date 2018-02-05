/*
 * Copyright (c) 2005 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
 * USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
 */
#ifndef __XFS_H__
#define __XFS_H__

#if defined(__linux__)
#include <xfs/linux.h>
#elif defined(__FreeBSD__)
#include <xfs/freebsd.h>
#elif defined(__FreeBSD_kernel__)
#include <xfs/gnukfreebsd.h>
#elif defined(__APPLE__)
#include <xfs/darwin.h>
#else
# error unknown platform... have fun porting!
#endif

/*
 * make sure that any user of the xfs headers has a 64bit off_t type
 */
extern int xfs_assert_largefile[sizeof(off_t)-8];

/*
 * sparse kernel source annotations
 */
#ifndef __user
#define __user
#endif

/*
 * kernel struct packing shortcut
 */
#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include <xfs/xfs_types.h>
#include <xfs/xfs_fs.h>

#endif	/* __XFS_H__ */
