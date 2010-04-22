/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * yaffs_mtdif.c  NAND mtd wrapper functions.
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __YAFFS_MTDIF2_H__
#define __YAFFS_MTDIF2_H__

#include "yaffs_guts.h"
int nandmtd2_WriteChunkWithTagsToNAND(yaffs_Device * dev, int chunkInNAND,
				      const __u8 * data,
				      const yaffs_ExtendedTags * tags);
int nandmtd2_ReadChunkWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND,
				       __u8 * data, yaffs_ExtendedTags * tags);
int nandmtd2_MarkNANDBlockBad(struct yaffs_DeviceStruct *dev, int blockNo);
int nandmtd2_QueryNANDBlock(struct yaffs_DeviceStruct *dev, int blockNo,
			    yaffs_BlockState * state, int *sequenceNumber);

#endif
