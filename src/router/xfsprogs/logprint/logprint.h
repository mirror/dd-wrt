// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef LOGPRINT_H
#define LOGPRINT_H

/* command line flags */
extern int	print_data;
extern int	print_only_data;
extern int	print_inode;
extern int	print_quota;
extern int	print_buffer;
extern int	print_transactions;
extern int	print_overwrite;
extern int	print_no_data;
extern int	print_no_print;

/* exports */
extern void xlog_print_lseek(struct xlog *, int, xfs_daddr_t, int);

extern void xfs_log_copy(struct xlog *, int, char *);
extern void xfs_log_dump(struct xlog *, int, int);
extern void xfs_log_print(struct xlog *, int, int);
extern void xfs_log_print_trans(struct xlog *, int);

extern void print_xlog_record_line(void);
extern void print_xlog_op_line(void);
extern void print_stars(void);

extern struct xfs_inode_log_format *
	xfs_inode_item_format_convert(char *, uint, struct xfs_inode_log_format *);

extern int xlog_print_trans_efi(char **ptr, uint src_len, int continued);
extern void xlog_recover_print_efi(struct xlog_recover_item *item);
extern int xlog_print_trans_efd(char **ptr, uint len);
extern void xlog_recover_print_efd(struct xlog_recover_item *item);

extern int xlog_print_trans_rui(char **ptr, uint src_len, int continued);
extern void xlog_recover_print_rui(struct xlog_recover_item *item);
extern int xlog_print_trans_rud(char **ptr, uint len);
extern void xlog_recover_print_rud(struct xlog_recover_item *item);

extern int xlog_print_trans_cui(char **ptr, uint src_len, int continued);
extern void xlog_recover_print_cui(struct xlog_recover_item *item);
extern int xlog_print_trans_cud(char **ptr, uint len);
extern void xlog_recover_print_cud(struct xlog_recover_item *item);

extern int xlog_print_trans_bui(char **ptr, uint src_len, int continued);
extern void xlog_recover_print_bui(struct xlog_recover_item *item);
extern int xlog_print_trans_bud(char **ptr, uint len);
extern void xlog_recover_print_bud(struct xlog_recover_item *item);

#endif	/* LOGPRINT_H */
