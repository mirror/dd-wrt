/*
 * dproto.h -- Darwin function prototypes for libproc-based lsof
 *
 * The _PROTOTYPE macro is defined in the common proto.h.
 */

/*
 * Portions Copyright 2005 Apple Computer, Inc.  All rights reserved.
 *
 * Copyright 2005 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Allan Nathanson, Apple Computer, Inc., and Victor A.
 * Abell, Purdue University.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors, nor Apple Computer, Inc. nor Purdue University
 *    are responsible for any consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either
 *    by explicit claim or by omission.  Credit to the authors, Apple
 *    Computer, Inc. and Purdue University must appear in documentation
 *    and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

/*
 * $Id: dproto.h,v 1.6 2012/04/10 16:41:04 abe Exp abe $
 */

extern void enter_file_info(struct lsof_context *ctx,
                            struct proc_fileinfo *pfi);
extern void enter_vnode_info(struct lsof_context *ctx,
                             struct vnode_info_path *vip);
extern void err2nm(struct lsof_context *ctx, char *pfx);
extern int is_file_named(struct lsof_context *ctx, char *p, int cd);
extern void process_atalk(struct lsof_context *ctx, int pid, int32_t fd);
extern void process_fsevents(struct lsof_context *ctx, int pid, int32_t fd);
extern void process_kqueue(struct lsof_context *ctx, int pid, int32_t fd);
extern void process_pipe(struct lsof_context *ctx, int pid, int32_t fd);
extern void process_psem(struct lsof_context *ctx, int pid, int32_t fd);
extern void process_pshm(struct lsof_context *ctx, int pid, int32_t fd);
extern void process_socket(struct lsof_context *ctx, int pid, int32_t fd);
extern void process_vnode(struct lsof_context *ctx, int pid, int32_t fd);
#if defined(PROC_PIDLISTFILEPORTS)
extern void process_fileport_pipe(struct lsof_context *ctx, int pid,
                                  uint32_t fileport);
extern void process_fileport_pshm(struct lsof_context *ctx, int pid,
                                  uint32_t fileport);
extern void process_fileport_socket(struct lsof_context *ctx, int pid,
                                    uint32_t fileport);
extern void process_fileport_vnode(struct lsof_context *ctx, int pid,
                                   uint32_t fileport);
#endif /* PROC_PIDLISTFILEPORTS */
