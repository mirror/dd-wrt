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

const char *yaffs_checkptrw_c_version =
    "$Id: yaffs_checkptrw.c,v 1.4 2006/05/23 19:08:41 charles Exp $";


#include "yaffs_checkptrw.h"


static int yaffs_CheckpointSpaceOk(yaffs_Device *dev)
{

	int blocksAvailable = dev->nErasedBlocks - dev->nReservedBlocks;
	
	T(YAFFS_TRACE_CHECKPOINT,
		(TSTR("checkpt blocks available = %d" TENDSTR),
		blocksAvailable));
		
	
	return (blocksAvailable <= 0) ? 0 : 1;
}



static int yaffs_CheckpointErase(yaffs_Device *dev)
{
	
	int i;
	

	if(!dev->eraseBlockInNAND)	
		return 0;
	T(YAFFS_TRACE_CHECKPOINT,(TSTR("checking blocks %d to %d"TENDSTR),
		dev->startBlock,dev->endBlock));
		
	for(i = dev->startBlock; i <= dev->endBlock; i++) {
		yaffs_BlockInfo *bi = &dev->blockInfo[i];
		if(bi->blockState == YAFFS_BLOCK_STATE_CHECKPOINT){
			T(YAFFS_TRACE_CHECKPOINT,(TSTR("erasing checkpt block %d"TENDSTR),i));
			if(dev->eraseBlockInNAND(dev,i)){
				bi->blockState = YAFFS_BLOCK_STATE_EMPTY;
				dev->nErasedBlocks++;
				dev->nFreeChunks += dev->nChunksPerBlock;
			}
			else {
				dev->markNANDBlockBad(dev,i);
				bi->blockState = YAFFS_BLOCK_STATE_DEAD;
			}
		}
	}
	
	dev->blocksInCheckpoint = 0;
	
	return 1;
}


static void yaffs_CheckpointFindNextErasedBlock(yaffs_Device *dev)
{
	int  i;
	int blocksAvailable = dev->nErasedBlocks - dev->nReservedBlocks;
		
	if(dev->checkpointNextBlock >= 0 &&
	   dev->checkpointNextBlock <= dev->endBlock &&
	   blocksAvailable > 0){
	
		for(i = dev->checkpointNextBlock; i <= dev->endBlock; i++){
			yaffs_BlockInfo *bi = &dev->blockInfo[i];
			if(bi->blockState == YAFFS_BLOCK_STATE_EMPTY){
				dev->checkpointNextBlock = i + 1;
				dev->checkpointCurrentBlock = i;
				T(YAFFS_TRACE_CHECKPOINT,(TSTR("allocating checkpt block %d"TENDSTR),i));
				return;
			}
		}
	}
	T(YAFFS_TRACE_CHECKPOINT,(TSTR("out of checkpt blocks"TENDSTR)));
	
	dev->checkpointNextBlock = -1;
	dev->checkpointCurrentBlock = -1;
}

static void yaffs_CheckpointFindNextCheckpointBlock(yaffs_Device *dev)
{
	int  i;
	yaffs_ExtendedTags tags;
	
	if(dev->blocksInCheckpoint < dev->checkpointMaxBlocks) 
		for(i = dev->checkpointNextBlock; i <= dev->endBlock; i++){
			int chunk = i * dev->nChunksPerBlock;

			dev->readChunkWithTagsFromNAND(dev,chunk,NULL,&tags);
						      
			if(tags.sequenceNumber == YAFFS_SEQUENCE_CHECKPOINT_DATA){
				/* Right kind of block */
				dev->checkpointNextBlock = tags.objectId;
				dev->checkpointCurrentBlock = i;
				dev->checkpointBlockList[dev->blocksInCheckpoint] = i;
				dev->blocksInCheckpoint++;
				T(YAFFS_TRACE_CHECKPOINT,(TSTR("found checkpt block %d"TENDSTR),i));
				return;
			}
		}

	T(YAFFS_TRACE_CHECKPOINT,(TSTR("found no more checkpt blocks"TENDSTR)));

	dev->checkpointNextBlock = -1;
	dev->checkpointCurrentBlock = -1;
}


int yaffs_CheckpointOpen(yaffs_Device *dev, int forWriting)
{
	
	/* Got the functions we need? */
	if (!dev->writeChunkWithTagsToNAND ||
	    !dev->readChunkWithTagsFromNAND ||
	    !dev->eraseBlockInNAND ||
	    !dev->markNANDBlockBad)
		return 0;

	if(forWriting && !yaffs_CheckpointSpaceOk(dev))
		return 0;
			
	if(!dev->checkpointBuffer)
		dev->checkpointBuffer = YMALLOC_DMA(dev->nBytesPerChunk);
	if(!dev->checkpointBuffer)
		return 0;

	
	dev->checkpointPageSequence = 0;
	
	dev->checkpointOpenForWrite = forWriting;
	
	dev->checkpointByteCount = 0;
	dev->checkpointCurrentBlock = -1;
	dev->checkpointCurrentChunk = -1;
	dev->checkpointNextBlock = dev->startBlock;
	
	/* Erase all the blocks in the checkpoint area */
	if(forWriting){
		memset(dev->checkpointBuffer,0,dev->nBytesPerChunk);
		dev->checkpointByteOffset = 0;
		return yaffs_CheckpointErase(dev);
		
		
	} else {
		int i;
		/* Set to a value that will kick off a read */
		dev->checkpointByteOffset = dev->nBytesPerChunk;
		/* A checkpoint block list of 1 checkpoint block per 16 block is (hopefully)
		 * going to be way more than we need */
		dev->blocksInCheckpoint = 0;
		dev->checkpointMaxBlocks = (dev->endBlock - dev->startBlock)/16 + 2;
		dev->checkpointBlockList = YMALLOC(sizeof(int) * dev->checkpointMaxBlocks);
		for(i = 0; i < dev->checkpointMaxBlocks; i++)
			dev->checkpointBlockList[i] = -1;
	}
	
	return 1;
}

static int yaffs_CheckpointFlushBuffer(yaffs_Device *dev)
{

	int chunk;

	yaffs_ExtendedTags tags;
	
	if(dev->checkpointCurrentBlock < 0){
		yaffs_CheckpointFindNextErasedBlock(dev);
		dev->checkpointCurrentChunk = 0;
	}
	
	if(dev->checkpointCurrentBlock < 0)
		return 0;
	
	tags.chunkDeleted = 0;
	tags.objectId = dev->checkpointNextBlock; /* Hint to next place to look */
	tags.chunkId = dev->checkpointPageSequence + 1;
	tags.sequenceNumber =  YAFFS_SEQUENCE_CHECKPOINT_DATA;
	tags.byteCount = dev->nBytesPerChunk;
	if(dev->checkpointCurrentChunk == 0){
		/* First chunk we write for the block? Set block state to
		   checkpoint */
		yaffs_BlockInfo *bi = &dev->blockInfo[dev->checkpointCurrentBlock];
		bi->blockState = YAFFS_BLOCK_STATE_CHECKPOINT;
		dev->blocksInCheckpoint++;
	}
	
	chunk = dev->checkpointCurrentBlock * dev->nChunksPerBlock + dev->checkpointCurrentChunk;
		
	dev->writeChunkWithTagsToNAND(dev,chunk,dev->checkpointBuffer,&tags);
	dev->checkpointByteOffset = 0;
	dev->checkpointPageSequence++;	   
	dev->checkpointCurrentChunk++;
	if(dev->checkpointCurrentChunk >= dev->nChunksPerBlock){
		dev->checkpointCurrentChunk = 0;
		dev->checkpointCurrentBlock = -1;
	}
	memset(dev->checkpointBuffer,0,dev->nBytesPerChunk);
	
	return 1;
}


int yaffs_CheckpointWrite(yaffs_Device *dev,const void *data, int nBytes)
{
	int i=0;
	int ok = 1;

	
	__u8 * dataBytes = (__u8 *)data;
	
	

	if(!dev->checkpointBuffer)
		return 0;

	while(i < nBytes && ok) {
		

		
		 dev->checkpointBuffer[dev->checkpointByteOffset] = *dataBytes ;
		dev->checkpointByteOffset++;
		i++;
		dataBytes++;
		dev->checkpointByteCount++;
		
		
		if(dev->checkpointByteOffset < 0 ||
		   dev->checkpointByteOffset >= dev->nBytesPerChunk) 
			ok = yaffs_CheckpointFlushBuffer(dev);

	}
	
	return 	i;
}

int yaffs_CheckpointRead(yaffs_Device *dev, void *data, int nBytes)
{
	int i=0;
	int ok = 1;
	yaffs_ExtendedTags tags;

	
	int chunk;

	__u8 *dataBytes = (__u8 *)data;
		
	if(!dev->checkpointBuffer)
		return 0;

	while(i < nBytes && ok) {
	
	
		if(dev->checkpointByteOffset < 0 ||
		   dev->checkpointByteOffset >= dev->nBytesPerChunk) {
		   
		   	if(dev->checkpointCurrentBlock < 0){
				yaffs_CheckpointFindNextCheckpointBlock(dev);
				dev->checkpointCurrentChunk = 0;
			}
			
			if(dev->checkpointCurrentBlock < 0)
				ok = 0;
			else {
			
				chunk = dev->checkpointCurrentBlock * dev->nChunksPerBlock + 
				          dev->checkpointCurrentChunk;

	   			/* read in the next chunk */
	   			/* printf("read checkpoint page %d\n",dev->checkpointPage); */
				dev->readChunkWithTagsFromNAND(dev, chunk, 
							       dev->checkpointBuffer,
							      &tags);
						      
				if(tags.chunkId != (dev->checkpointPageSequence + 1) ||
				   tags.sequenceNumber != YAFFS_SEQUENCE_CHECKPOINT_DATA)
				   ok = 0;

				dev->checkpointByteOffset = 0;
				dev->checkpointPageSequence++;
				dev->checkpointCurrentChunk++;
			
				if(dev->checkpointCurrentChunk >= dev->nChunksPerBlock)
					dev->checkpointCurrentBlock = -1;
			}
		}
		
		if(ok){
			*dataBytes = dev->checkpointBuffer[dev->checkpointByteOffset];
			dev->checkpointByteOffset++;
			i++;
			dataBytes++;
			dev->checkpointByteCount++;
		}
	}
	
	return 	i;
}

int yaffs_CheckpointClose(yaffs_Device *dev)
{

	if(dev->checkpointOpenForWrite){	
		if(dev->checkpointByteOffset != 0)
			yaffs_CheckpointFlushBuffer(dev);
	} else {
		int i;
		for(i = 0; i < dev->blocksInCheckpoint && dev->checkpointBlockList[i] >= 0; i++){
			yaffs_BlockInfo *bi = &dev->blockInfo[dev->checkpointBlockList[i]];
			if(bi->blockState == YAFFS_BLOCK_STATE_EMPTY)
				bi->blockState = YAFFS_BLOCK_STATE_CHECKPOINT;
			else {
				// Todo this looks odd...
			}
		}
		YFREE(dev->checkpointBlockList);
		dev->checkpointBlockList = NULL;
	}

	dev->nFreeChunks -= dev->blocksInCheckpoint * dev->nChunksPerBlock;
	dev->nErasedBlocks -= dev->blocksInCheckpoint;

		
	T(YAFFS_TRACE_CHECKPOINT,(TSTR("checkpoint byte count %d" TENDSTR),
			dev->checkpointByteCount));
			
	if(dev->checkpointBuffer){
		/* free the buffer */	
		YFREE(dev->checkpointBuffer);
		dev->checkpointBuffer = NULL;
		return 1;
	}
	else
		return 0;
	
}

int yaffs_CheckpointInvalidateStream(yaffs_Device *dev)
{
	/* Erase the first checksum block */

	T(YAFFS_TRACE_CHECKPOINT,(TSTR("checkpoint invalidate"TENDSTR)));

	if(!yaffs_CheckpointSpaceOk(dev))
		return 0;

	return yaffs_CheckpointErase(dev);
}



