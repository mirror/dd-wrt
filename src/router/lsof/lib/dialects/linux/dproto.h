/*
 * dproto.h - Linux function prototypes for /proc-based lsof
 *
 * The _PROTOTYPE macro is defined in the common proto.h.
 */

/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
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
 * $Id: dproto.h,v 1.9 2013/01/02 17:02:36 abe Exp $
 */

#if defined(HASSELINUX)
extern int enter_cntx_arg(struct lsof_context *ctx, char *cnxt);
#endif /* defined(HASSELINUX) */

extern int get_fields(struct lsof_context *ctx, char *ln, char *sep, char ***fr,
                      int *eb, int en);
extern void get_locks(struct lsof_context *ctx, char *p);
extern void clean_locks(struct lsof_context *ctx);
extern void clean_ax25(struct lsof_context *ctx);
extern void clean_icmp(struct lsof_context *ctx);
extern void clean_ipx(struct lsof_context *ctx);
extern void clean_netlink(struct lsof_context *ctx);
extern void clean_pack(struct lsof_context *ctx);
extern void clean_raw(struct lsof_context *ctx);
extern void clean_sctp(struct lsof_context *ctx);
extern void clean_unix(struct lsof_context *ctx);
extern void clean_tcpudp(struct lsof_context *ctx, int free_array);
#if defined(HASIPv6)
extern void clean_raw6(struct lsof_context *ctx);
extern void clean_tcpudp6(struct lsof_context *ctx, int free_array);
#endif
extern int is_file_named(struct lsof_context *ctx, int ty, char *p,
                         struct mounts *mp, int cd);
extern int make_proc_path(struct lsof_context *ctx, char *pp, int lp, char **np,
                          int *npl, char *sf);
extern FILE *open_proc_stream(struct lsof_context *ctx, char *p, char *mode,
                              char **buf, size_t *sz, int act);
extern void process_proc_node(struct lsof_context *ctx, char *p, char *pbr,
                              struct stat *s, int ss, struct stat *l, int ls);
extern void process_proc_sock(struct lsof_context *ctx, char *p, char *pbr,
                              struct stat *s, int ss, struct stat *l, int ls);
extern void set_net_paths(struct lsof_context *ctx, char *p, int pl);
extern void refresh_socket_info(struct lsof_context *ctx);