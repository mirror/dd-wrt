/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2010 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __YAFFS_CHECKPTRW_H__
#define __YAFFS_CHECKPTRW_H__

#include "yaffs_guts.h"

int yaffs2_checkpt_open(yaffs_dev_t *dev, int forWriting);

int yaffs2_checkpt_wr(yaffs_dev_t *dev, const void *data, int n_bytes);

int yaffs2_checkpt_rd(yaffs_dev_t *dev, void *data, int n_bytes);

int yaffs2_get_checkpt_sum(yaffs_dev_t *dev, __u32 *sum);

int yaffs_checkpt_close(yaffs_dev_t *dev);

int yaffs2_checkpt_invalidate_stream(yaffs_dev_t *dev);


#endif
