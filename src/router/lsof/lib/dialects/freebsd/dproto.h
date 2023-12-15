/*
 * dproto.h - FreeBSD function prototypes for lsof
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
 * $Id: dproto.h,v 1.15 2018/02/14 14:26:03 abe Exp $
 */

#if !defined(N_UNIX)
extern char *get_nlist_path(struct lsof_context *ctx, int ap);
#endif /* !defined(N_UNIX) */

extern int is_file_named(struct lsof_context *ctx, char *p, int cd);
extern void process_vnode(struct lsof_context *ctx, struct kinfo_file *kf,
                          struct xfile *xfile, struct lock_list *locks);
extern void process_socket(struct lsof_context *ctx, struct kinfo_file *kf,
                           struct pcb_lists *pcbs);
extern struct l_vfs *readvfs(struct lsof_context *ctx, uint64_t fsid,
                             const char *path);
extern struct pcb_lists *read_pcb_lists(void);
extern void free_pcb_lists(struct pcb_lists *pcb_lists);
extern int cmp_kinfo_lockf(const void *a, const void *b);

extern void process_pts(struct lsof_context *ctx, struct kinfo_file *kf);

#if defined(KF_TYPE_EVENTFD)
extern void process_eventfd(struct lsof_context *ctx, struct kinfo_file *kf);
#endif /* defined(KF_TYPE_EVENTFD) */

#if defined(HASKQUEUE)
extern void process_kf_kqueue(struct lsof_context *ctx, struct kinfo_file *kf,
                              KA_T ka);
#endif /* defined(HASKQUEUE) */

extern void process_pipe(struct lsof_context *ctx, struct kinfo_file *kf,
                         KA_T pa);
extern void process_shm(struct lsof_context *ctx, struct kinfo_file *kf);
extern void process_procdesc(struct lsof_context *ctx, struct kinfo_file *kf);

#if defined(HAS9660FS)
extern int read_iso_node(struct vnode *v, dev_t *d, int *dd, INODETYPE *ino,
                         long *nl, SZOFFTYPE *sz);
#endif /* defined(HAS9660FS) */

#if defined(HASMSDOSFS)
extern int read_msdos_node(struct vnode *v, dev_t *d, int *dd, INODETYPE *ino,
                           long *nl, SZOFFTYPE *sz);
#endif /* defined(HASMSDOSFS) */
