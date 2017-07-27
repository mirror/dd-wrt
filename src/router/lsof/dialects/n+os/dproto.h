/*
 * dproto.h - NEXTSTEP and OPENSTEP function prototypes for lsof
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
 * $Id: dproto.h,v 1.6 2001/08/09 11:44:07 abe Exp $
 */


#if	STEPV>=31
_PROTOTYPE(extern void clr_svnc,(void));
#endif	/* STEPV>=31 */

_PROTOTYPE(extern int is_file_named,(char *p, int cd));

#if     defined(HAS_AFS)
_PROTOTYPE(extern struct vnode *alloc_vcache,(void));
_PROTOTYPE(extern void ckAFSsym,(struct nlist *nl));
_PROTOTYPE(extern int hasAFS,(struct vnode *vp));
_PROTOTYPE(extern void process_socket,(KA_T vp));
_PROTOTYPE(extern int readafsnode,(caddr_t va, struct vnode *v, struct afsnode *
an));
#endif  /* defined(HAS_AFS) */
