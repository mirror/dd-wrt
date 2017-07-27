/*
 * dproto.h - pstat-based HP-UX function prototypes for lsof
 *
 * The _PROTOTYPE macro is defined in the common proto.h.
 */


/*
 * Copyright 1999 Purdue Research Foundation, West Lafayette, Indiana
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
 * $Id: dproto.h,v 1.5 2008/10/21 16:17:50 abe Exp $
 */


_PROTOTYPE(extern int get_max_fd,(void));
_PROTOTYPE(extern int is_file_named,(char *p, int cd));
_PROTOTYPE(extern void process_finfo,(struct pst_filedetails *pd, struct pst_fid *opfid, struct psfileid *psfid, KA_T na));
_PROTOTYPE(extern void process_socket,(struct pst_fileinfo2 *f,
				       struct pst_socket *s));
_PROTOTYPE(extern void process_stream,(struct pst_fileinfo2 *f, int ckscko));
_PROTOTYPE(extern KA_T read_det,(struct pst_fid *ki, uint32_t hf, uint32_t lf,
				 uint32_t hn, uint32_t ln,
				 struct pst_filedetails *pd));
_PROTOTYPE(extern struct pst_socket *read_sock,(struct pst_fileinfo2 *f));

#if	defined(HASIPv6)
_PROTOTYPE(extern struct hostent *gethostbyname2,(char *nm, int proto));
#endif	/* defined(HASIPv6) */

#if	defined(HASVXFS)
_PROTOTYPE(extern int read_vxnode,(struct vnode *v, struct l_vfs *vfs, dev_t *dev));
#endif	/* defined(HASVXFS) */

_PROTOTYPE(extern void scanmnttab,(void));
