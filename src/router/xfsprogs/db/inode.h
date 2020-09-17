// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const struct field	inode_a_flds[];
extern const struct field	inode_core_flds[];
extern const struct field	inode_v3_flds[];
extern const struct field	inode_flds[];
extern const struct field	inode_crc_flds[];
extern const struct field	inode_hfld[];
extern const struct field	inode_crc_hfld[];
extern const struct field	inode_u_flds[];
extern const struct field	timestamp_flds[];

extern int	fp_dinode_fmt(void *obj, int bit, int count, char *fmtstr,
			      int size, int arg, int base, int array);
extern int	inode_a_size(void *obj, int startoff, int idx);
extern void	inode_init(void);
extern typnm_t	inode_next_type(void);
extern int	inode_size(void *obj, int startoff, int idx);
extern int	inode_u_size(void *obj, int startoff, int idx);
extern void	xfs_inode_set_crc(struct xfs_buf *);
extern void	set_cur_inode(xfs_ino_t ino);
