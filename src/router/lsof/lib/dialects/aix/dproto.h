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

#if !defined(DPROTO_H)
#    define DPROTO_H

#    if defined(HAS_AFS)
extern struct vnode *alloc_vcache(void);
extern int hasAFS(struct lsof_context *ctx, struct vnode *vp);
extern int readafsnode(struct lsof_context *ctx, KA_T va, struct vnode *v,
                       struct afsnode *an);
#    endif /* defined(HAS_AFS) */

#    if defined(HAS_JFS2)
extern int readj2lino(struct lsof_context *ctx, struct gnode *ga, struct l_ino *li);
#    endif /* defined(HAS_JFS2) */

extern int getchan(char *p);
extern int is_file_named(struct lsof_context *ctx, char *p, enum vtype ty, chan_t ch, int ic);
extern char isglocked(struct lsof_context *ctx, struct gnode *ga);
extern int readlino(struct lsof_context *ctx, struct gnode *ga, struct l_ino *li);
extern struct l_vfs *readvfs(struct lsof_context *ctx, struct vnode *vn);

#    if AIXV >= 4200
extern void process_shmt(struct lsof_context *ctx, KA_T sa);
#    endif /* AIV>=4200 */

#    if defined(HASDCACHE) && AIXV >= 4140
extern void clr_sect(struct lsof_context *ctx);
extern int rw_clone_sect(struct lsof_context *ctx, int m);
#    endif /* defined(HASDCACHE) && AIXV>=4140 */

#endif /* !defined(DPROTO_H) */
