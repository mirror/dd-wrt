/*
 * dproto.h - AIX function prototypes for lsof
 *
 * The _PROTOTYPE macro is defined in the common proto.h.
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


/*
 * $Id: dproto.h,v 1.4 2004/03/10 23:49:13 abe Exp $
 */


#if	!defined(DPROTO_H)
#define	DPROTO_H

# if     defined(HAS_AFS)
_PROTOTYPE(extern struct vnode *alloc_vcache,(void));
_PROTOTYPE(extern int hasAFS,(struct vnode *vp));
_PROTOTYPE(extern int readafsnode,(KA_T va, struct vnode *v, struct afsnode *an));
# endif  /* defined(HAS_AFS) */

# if	defined(HAS_JFS2)
_PROTOTYPE(extern int readj2lino,(struct gnode *ga, struct l_ino *li));
# endif	/* defined(HAS_JFS2) */

_PROTOTYPE(extern int getchan,(char *p));
_PROTOTYPE(extern int is_file_named,(char *p, enum vtype ty, chan_t ch, int ic));
_PROTOTYPE(extern char isglocked,(struct gnode *ga));
_PROTOTYPE(extern int readlino,(struct gnode *ga, struct l_ino *li));
_PROTOTYPE(extern struct l_vfs *readvfs,(struct vnode *vn));

# if	AIXV>=4200
_PROTOTYPE(extern void process_shmt,(KA_T sa));
# endif	/* AIV>=4200 */

# if	defined(HASDCACHE) && AIXV>=4140
_PROTOTYPE(extern void clr_sect,(void));
_PROTOTYPE(extern int rw_clone_sect,(int m));
# endif	/* defined(HASDCACHE) && AIXV>=4140 */

#endif	/* !defined(DPROTO_H) */
