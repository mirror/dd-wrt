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
 
const char *yaffs_nand_c_version =
    "$Id: yaffs_nand.c,v 1.1 2006/05/08 10:13:34 charles Exp $";

#include "yaffs_nand.h"
#include "yaffs_tagscompat.h"
#include "yaffs_tagsvalidity.h"


static int yaffs_ReadCDataWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND,
					   __u8 * buffer,
					   yaffs_ExtendedTags * tags)
{
	chunkInNAND -= dev->chunkOffset;

	dev->nPageReads++;

	if (dev->readChunkWithTagsFromNAND)
		return dev->readChunkWithTagsFromNAND(dev, chunkInNAND, buffer,
						      tags);
	else
		return yaffs_TagsCompatabilityReadChunkWithTagsFromNAND(dev,
									chunkInNAND,
									buffer,
									tags);
}

int yaffs_ReadChunkWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND,
				    __u8 * buffer,
				    yaffs_ExtendedTags * tags)
{
	int retVal = yaffs_ReadCDataWithTagsFromNAND(dev, chunkInNAND,
						     buffer, tags);
	if (retVal == YAFFS_FAIL) {
		yaffs_HandleReadDataError(dev, chunkInNAND, 0);

		/* retry reading - may succeed in case of soft error */
		retVal = yaffs_ReadCDataWithTagsFromNAND(dev, chunkInNAND,
							 buffer, tags);
		if (retVal == YAFFS_FAIL) {
			retVal = yaffs_ReadCDataWithTagsFromNAND(dev,
								 chunkInNAND,
								 buffer, tags);
		}
		if (retVal == YAFFS_OK) {
			T(YAFFS_TRACE_ERROR,
			  (TSTR
			   ("**>>succeeded to read chunk %d after read error"
			    TENDSTR), chunkInNAND - dev->chunkOffset));
		}
	}
	return retVal;
}

int yaffs_WriteChunkWithTagsToNAND(yaffs_Device * dev,
						   int chunkInNAND,
						   const __u8 * buffer,
						   yaffs_ExtendedTags * tags)
{
	chunkInNAND -= dev->chunkOffset;

	
	if (tags) {
		tags->sequenceNumber = dev->sequenceNumber;
		tags->chunkUsed = 1;
		if (!yaffs_ValidateTags(tags)) {
			T(YAFFS_TRACE_ERROR,
			  (TSTR("Writing uninitialised tags" TENDSTR)));
			YBUG();
		}
		T(YAFFS_TRACE_WRITE,
		  (TSTR("Writing chunk %d tags %d %d" TENDSTR), chunkInNAND,
		   tags->objectId, tags->chunkId));
	} else {
		T(YAFFS_TRACE_ERROR, (TSTR("Writing with no tags" TENDSTR)));
		YBUG();
	}

	dev->nPageWrites++;

	if (dev->writeChunkWithTagsToNAND)
		return dev->writeChunkWithTagsToNAND(dev, chunkInNAND, buffer,
						     tags);
	else
		return yaffs_TagsCompatabilityWriteChunkWithTagsToNAND(dev,
								       chunkInNAND,
								       buffer,
								       tags);
}

int yaffs_MarkBlockBad(yaffs_Device * dev, int blockNo)
{
	blockNo -= dev->blockOffset;

;
	if (dev->markNANDBlockBad)
		return dev->markNANDBlockBad(dev, blockNo);
	else
		return yaffs_TagsCompatabilityMarkNANDBlockBad(dev, blockNo);
}

int yaffs_QueryInitialBlockState(yaffs_Device * dev,
						 int blockNo,
						 yaffs_BlockState * state,
						 unsigned *sequenceNumber)
{
	blockNo -= dev->blockOffset;

	if (dev->queryNANDBlock)
		return dev->queryNANDBlock(dev, blockNo, state, sequenceNumber);
	else
		return yaffs_TagsCompatabilityQueryNANDBlock(dev, blockNo,
							     state,
							     sequenceNumber);
}


int yaffs_EraseBlockInNAND(struct yaffs_DeviceStruct *dev,
				  int blockInNAND)
{
	int result;

	blockInNAND -= dev->blockOffset;


	dev->nBlockErasures++;
	result = dev->eraseBlockInNAND(dev, blockInNAND);

	/* If at first we don't succeed, try again *once*.*/
	if (!result)
		result = dev->eraseBlockInNAND(dev, blockInNAND);	
	return result;
}

int yaffs_InitialiseNAND(struct yaffs_DeviceStruct *dev)
{
	return dev->initialiseNAND(dev);
}


 
