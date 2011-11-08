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

#include <xfs/libxlog.h>

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
extern char *trans_type[];

extern void xlog_print_lseek(xlog_t *, int, xfs_daddr_t, int);

extern void xfs_log_copy(xlog_t *, int, char *);
extern void xfs_log_dump(xlog_t *, int, int);
extern void xfs_log_print(xlog_t *, int, int);
extern void xfs_log_print_trans(xlog_t *, int);

extern void print_xlog_record_line(void);
extern void print_xlog_op_line(void);
extern void print_stars(void);

extern xfs_inode_log_format_t *
	xfs_inode_item_format_convert(char *, uint, xfs_inode_log_format_t *);
extern int xfs_efi_copy_format(char *, uint, xfs_efi_log_format_t *);

#endif	/* LOGPRINT_H */
