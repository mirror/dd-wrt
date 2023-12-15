/*
 * dproto.h - Solaris function prototypes for lsof
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
 * $Id: dproto.h,v 1.21 2010/01/18 19:03:54 abe Exp $
 */

#if defined(HASVXFSUTIL)
extern int access_vxfs_ioffsets(void);
#endif /* defined(HASVXFSUTIL) */

extern void completevfs(struct lsof_context *ctx, struct l_vfs *vfs,
                        dev_t *dev);

#if defined(HAS_LIBCTF)
extern int CTF_getmem(struct lsof_context *ctx, ctf_file_t *f, const char *mod,
                      const char *ty, CTF_member_t *mem);
extern void CTF_init(struct lsof_context *ctx, int *i, char *t,
                     CTF_request_t *r);
extern int CTF_memCB(const char *name, ctf_id_t id, ulong_t offset, void *arg);
#endif /* defined(HAS_LIBCTF) */

extern int is_file_named(struct lsof_context *ctx, char *p, int nt,
                         enum vtype vt, int ps);
extern struct l_vfs *readvfs(struct lsof_context *ctx, KA_T ka, struct vfs *la,
                             struct vnode *lv);
extern int vop2ty(struct lsof_context *ctx, struct vnode *vp, int fx);

#if defined(HAS_AFS)
extern struct vnode *alloc_vcache(void);
extern void ckAFSsym(struct nlist *nl);
extern int hasAFS(struct vnode *vp);
extern int readafsnode(KA_T va, struct vnode *v, struct afsnode *an);
#endif /* defined(HAS_AFS) */

#if defined(HASDCACHE)
extern int rw_clone_sect(struct lsof_context *ctx, int m);
extern void clr_sect(struct lsof_context *ctx);
extern int rw_pseudo_sect(struct lsof_context *ctx, int m);
#endif /* defined(HASDCACHE) */

#if defined(HASIPv6)
extern struct hostent *gethostbyname2(const char *nm, int proto);
#endif /* defined(HASIPv6) */

#if defined(HAS_V_PATH)
extern int print_v_path(struct lsof_context *ctx, struct lfile *lf);
extern void read_v_path(struct lsof_context *ctx, KA_T ka, char *rb,
                        size_t rbl);
#endif /* defined(HAS_V_PATH) */

#if defined(HASVXFS)
extern int read_vxnode(struct lsof_context *ctx, KA_T va, struct vnode *v,
                       struct l_vfs *vfs, int fx, struct l_ino *li,
                       KA_T *vnops);
#    if defined(HASVXFSRNL)
extern int print_vxfs_rnl_path(struct lfile *lf);
#    endif /* defined(HASVXFSRNL) */
#endif     /* defined(HASVXFS) */

#if defined(HASZONES)
extern int enter_zone_arg(struct lsof_context *ctx, char *zn);
#endif /* defined(HASZONES) */

extern void close_kvm(struct lsof_context *ctx);
extern void open_kvm(struct lsof_context *ctx);
extern void process_socket(struct lsof_context *ctx, KA_T sa, char *ty);

#if solaris >= 110000
extern int process_VSOCK(struct lsof_context *ctx, KA_T va, struct vnode *v,
                         struct sonode *so);
#endif /* solaris>=11000 */

extern void read_clone(struct lsof_context *ctx);

#if solaris < 20500
extern int get_max_fd(void);
#endif /* solaris<20500 */

#if defined(WILLDROPGID)
extern void restoregid(struct lsof_context *ctx);
#endif /* defined(WILLDROPGID) */
