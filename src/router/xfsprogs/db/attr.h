// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const field_t	attr_flds[];
extern const field_t	attr_hfld[];
extern const field_t	attr_blkinfo_flds[];
extern const field_t	attr_leaf_entry_flds[];
extern const field_t	attr_leaf_hdr_flds[];
extern const field_t	attr_leaf_map_flds[];
extern const field_t	attr_leaf_name_flds[];
extern const field_t	attr_node_entry_flds[];
extern const field_t	attr_node_hdr_flds[];

extern const field_t	attr3_flds[];
extern const field_t	attr3_hfld[];
extern const field_t	attr3_leaf_hdr_flds[];
extern const field_t	attr3_node_hdr_flds[];
extern const field_t	attr3_blkinfo_flds[];
extern const field_t	attr3_node_hdr_flds[];
extern const field_t	attr3_remote_crc_flds[];

extern int	attr_leaf_name_size(void *obj, int startoff, int idx);
extern int	attr_size(void *obj, int startoff, int idx);
extern void	xfs_attr3_set_crc(struct xfs_buf *bp);

extern const struct xfs_buf_ops xfs_attr3_db_buf_ops;
