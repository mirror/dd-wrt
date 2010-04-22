/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
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

#ifndef __YAFFS_NAND_H__
#define __YAFFS_NAND_H__
#include "yaffs_guts.h"



int yaffs_ReadChunkWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND,
					   __u8 * buffer,
					   yaffs_ExtendedTags * tags);

int yaffs_WriteChunkWithTagsToNAND(yaffs_Device * dev,
						   int chunkInNAND,
						   const __u8 * buffer,
						   yaffs_ExtendedTags * tags);

int yaffs_MarkBlockBad(yaffs_Device * dev, int blockNo);

int yaffs_QueryInitialBlockState(yaffs_Device * dev,
						 int blockNo,
						 yaffs_BlockState * state,
						 unsigned *sequenceNumber);

int yaffs_EraseBlockInNAND(struct yaffs_DeviceStruct *dev,
				  int blockInNAND);

int yaffs_InitialiseNAND(struct yaffs_DeviceStruct *dev);

#endif

