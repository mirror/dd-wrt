// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const struct field	disk_dquot_flds[];
extern const struct field	dqblk_flds[];
extern const struct field	dqblk_hfld[];

extern void	xfs_dquot_set_crc(struct xfs_buf *);
extern void	dquot_init(void);
