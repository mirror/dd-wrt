/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * yaffs_mtdif.h  NAND mtd interface wrappers
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 *
 * $Id: yaffs_mtdif.h,v 1.3 2005/08/11 01:07:43 marty Exp $
 */

#ifndef __YAFFS_MTDIF_H__
#define __YAFFS_MTDIF_H__

#include "yaffs_guts.h"

int nandmtd_WriteChunkToNAND(yaffs_Device * dev, int chunkInNAND,
			     const __u8 * data, const yaffs_Spare * spare);
int nandmtd_ReadChunkFromNAND(yaffs_Device * dev, int chunkInNAND, __u8 * data,
			      yaffs_Spare * spare);
int nandmtd_EraseBlockInNAND(yaffs_Device * dev, int blockNumber);
int nandmtd_InitialiseNAND(yaffs_Device * dev);
#endif
