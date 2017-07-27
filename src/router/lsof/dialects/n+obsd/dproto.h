/*
 * dproto.h - NetBSD and OpenBSD function prototypes for lsof
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
 * $Id: dproto.h,v 1.11 2005/08/08 19:53:24 abe Exp $
 */


#if	!defined(N_UNIX)
_PROTOTYPE(extern char *get_nlist_path,(int ap));
#endif	/* !defined(N_UNIX) */

_PROTOTYPE(extern int is_file_named,(char *p, int cd));
_PROTOTYPE(extern struct l_vfs *readvfs,(KA_T vm));

#if	defined(HAS_SYS_PIPE_H)
_PROTOTYPE(extern void process_pipe,(KA_T pa));
#endif	/* defined(HAS_SYS_PIPEH) */

#if	defined(HAS9660FS)
_PROTOTYPE(extern int read_iso_node,(struct vnode *v, dev_t *d, INODETYPE *ino, long *nl, SZOFFTYPE *sz));
#endif	/* defined(HAS9660FS) */

_PROTOTYPE(extern void process_socket,(KA_T sa));
