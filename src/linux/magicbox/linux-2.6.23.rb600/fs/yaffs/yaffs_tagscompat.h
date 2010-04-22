/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * yaffs_ramdisk.h: yaffs ram disk component
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * $Id: yaffs_tagscompat.h,v 1.2 2005/08/11 02:33:03 marty Exp $
 */

/* This provides a ram disk under yaffs.
 * NB this is not intended for NAND emulation.
 * Use this with dev->useNANDECC enabled, then ECC overheads are not required.
 */
#ifndef __YAFFS_TAGSCOMPAT_H__
#define __YAFFS_TAGSCOMPAT_H__

#include "yaffs_guts.h"
int yaffs_TagsCompatabilityWriteChunkWithTagsToNAND(yaffs_Device * dev,
						    int chunkInNAND,
						    const __u8 * data,
						    const yaffs_ExtendedTags *
						    tags);
int yaffs_TagsCompatabilityReadChunkWithTagsFromNAND(yaffs_Device * dev,
						     int chunkInNAND,
						     __u8 * data,
						     yaffs_ExtendedTags *
						     tags);
int yaffs_TagsCompatabilityMarkNANDBlockBad(struct yaffs_DeviceStruct *dev,
					    int blockNo);
int yaffs_TagsCompatabilityQueryNANDBlock(struct yaffs_DeviceStruct *dev,
					  int blockNo, yaffs_BlockState *
					  state, int *sequenceNumber);

#endif
