/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

extern xfs_inode_log_format_t *
	xfs_inode_item_format_convert(char *, uint, xfs_inode_log_format_t *);

extern int xlog_print_trans_efi(char **ptr, uint src_len, int continued);
extern void xlog_recover_print_efi(xlog_recover_item_t *item);
extern int xlog_print_trans_efd(char **ptr, uint len);
extern void xlog_recover_print_efd(xlog_recover_item_t *item);

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
