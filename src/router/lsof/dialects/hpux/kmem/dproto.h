/*
 * dproto.h - /dev/kmem-based HP-UX function prototypes for lsof
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
 * $Id: dproto.h,v 1.7 2000/12/04 14:26:14 abe Exp $
 */


#if	HPUXV>=800
_PROTOTYPE(extern void completevfs,(struct l_vfs *vfs, dev_t *dev, struct vfs *v));
#else
_PROTOTYPE(extern void completevfs,(struct l_vfs *vfs, dev_t *dev));
#endif	/* HPUXV>=800 */

_PROTOTYPE(extern int is_file_named,(char *p, int cd));
_PROTOTYPE(extern int get_max_fd,(void));

#if	defined(DTYPE_LLA)
_PROTOTYPE(extern void process_lla,(KA_T la));
#endif

_PROTOTYPE(extern struct l_vfs *readvfs,(struct vnode *lv));

#if	HPUXV>=1030
_PROTOTYPE(extern void process_stream_sock,(KA_T ip, KA_T pcb, char *pn, enum vtype vt));
_PROTOTYPE(extern int read_mi,(KA_T sh, KA_T *ip, KA_T *pcb, char **pn));
#endif	/* HPUXV>=1030 */

#if     defined(HAS_AFS)
_PROTOTYPE(extern struct vnode *alloc_vcache,(void));
_PROTOTYPE(extern void ckAFSsym,(struct nlist *nl));
_PROTOTYPE(extern int hasAFS,(struct vnode *vp));
_PROTOTYPE(extern int readafsnode,(KA_T va, struct vnode *v, struct afsnode *an));
#endif  /* defined(HAS_AFS) */

#if	defined(HASVXFS)
_PROTOTYPE(extern int read_vxnode,(struct vnode *v, struct l_vfs *vfs, dev_t *dev, int *devs, dev_t *rdev, int *rdevs));
#endif	/* defined(HASVXFS) */
