/*
 * Copyright (c) 2000-2001 Silicon Graphics, Inc.
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

extern const struct field	inode_a_flds[];
extern const struct field	inode_core_flds[];
extern const struct field	inode_flds[];
extern const struct field	inode_hfld[];
extern const struct field	inode_u_flds[];
extern const struct field	timestamp_flds[];

extern int	fp_dinode_fmt(void *obj, int bit, int count, char *fmtstr,
			      int size, int arg, int base, int array);
extern int	inode_a_size(void *obj, int startoff, int idx);
extern void	inode_init(void);
extern typnm_t	inode_next_type(void);
extern int	inode_size(void *obj, int startoff, int idx);
extern int	inode_u_size(void *obj, int startoff, int idx);
extern void	set_cur_inode(xfs_ino_t ino);
