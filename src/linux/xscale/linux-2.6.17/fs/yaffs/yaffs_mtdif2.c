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

// mtd interface for YAFFS2

const char *yaffs_mtdif2_c_version = "$Id: yaffs_mtdif2.c,v 1.6 2005/08/01 20:52:35 luc Exp $";
 
#include "yportenv.h"

#ifdef CONFIG_YAFFS_YAFFS2

#include "yaffs_mtdif2.h"

#include "linux/mtd/mtd.h"
#include "linux/types.h"
#include "linux/time.h"

#include "yaffs_packedtags2.h"




int nandmtd2_WriteChunkWithTagsToNAND(yaffs_Device *dev,int chunkInNAND,const __u8 *data, const yaffs_ExtendedTags *tags)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	size_t dummy;
	int retval = 0;


	loff_t addr = ((loff_t)chunkInNAND) * dev->nBytesPerChunk;

	yaffs_PackedTags2 pt;
	
	T(YAFFS_TRACE_MTD,(TSTR("nandmtd2_WriteChunkWithTagsToNAND chunk %d data %p tags %p" TENDSTR),chunkInNAND,data,tags));	

	if(tags)
	{
		yaffs_PackTags2(&pt,tags);
	}

	if(data && tags)
	{
		if(dev->useNANDECC)
		  retval = mtd->write_ecc(mtd,addr,dev->nBytesPerChunk,&dummy,data,(__u8 *)&pt,NULL);
		else
		  retval = mtd->write_ecc(mtd,addr,dev->nBytesPerChunk,&dummy,data,(__u8 *)&pt,NULL);
	}
	else
	{
	if(data)
		retval = mtd->write(mtd,addr,dev->nBytesPerChunk,&dummy,data);
	if(tags)
		retval = mtd->write_oob(mtd,addr,mtd->oobsize,&dummy,(__u8 *)&pt);
		
	}

    if (retval == 0)
    	return YAFFS_OK;
    else
        return YAFFS_FAIL;
}

int nandmtd2_ReadChunkWithTagsFromNAND(yaffs_Device *dev,int chunkInNAND, __u8 *data, yaffs_ExtendedTags *tags)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	size_t dummy;
        int retval = 0;
	

	loff_t addr = ((loff_t)chunkInNAND) * dev->nBytesPerChunk;
	
	yaffs_PackedTags2 pt;

	T(YAFFS_TRACE_MTD,(TSTR("nandmtd2_ReadChunkWithTagsToNAND chunk %d data %p tags %p" TENDSTR),chunkInNAND,data,tags));	

	if(data && tags)
	{
		if(dev->useNANDECC)
		{
    			retval = mtd->read_ecc(mtd,addr,dev->nBytesPerChunk,&dummy,data,dev->spareBuffer,NULL);            
		}
		else
		{
			retval = mtd->read_ecc(mtd,addr,dev->nBytesPerChunk,&dummy,data,dev->spareBuffer,NULL);
		}
		memcpy(&pt,dev->spareBuffer,sizeof(pt));
	}
	else
	{
	if(data)
		retval = mtd->read(mtd,addr,dev->nBytesPerChunk,&dummy,data);
	if(tags) {
		retval = mtd->read_oob(mtd,addr,mtd->oobsize,&dummy,dev->spareBuffer);
		memcpy(&pt, mtd->oobinfo.oobfree[0][0] + (char *)dev->spareBuffer, sizeof(pt));
	}
	}

    if(tags)
	yaffs_UnpackTags2(tags,&pt);
    
    if (retval == 0)
    	return YAFFS_OK;
    else
        return YAFFS_FAIL;
}

int nandmtd2_MarkNANDBlockBad(struct yaffs_DeviceStruct *dev, int blockNo)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	int retval;
	T(YAFFS_TRACE_MTD,(TSTR("nandmtd2_MarkNANDBlockBad %d" TENDSTR),blockNo));	
	
	
	retval = mtd->block_markbad(mtd,blockNo * dev->nChunksPerBlock * dev->nBytesPerChunk);

    if (retval == 0)
    	return YAFFS_OK;
    else
        return YAFFS_FAIL;

}

int nandmtd2_QueryNANDBlock(struct yaffs_DeviceStruct *dev, int blockNo, yaffs_BlockState *state, int *sequenceNumber)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	int retval;
	
	T(YAFFS_TRACE_MTD,(TSTR("nandmtd2_QueryNANDBlock %d" TENDSTR),blockNo));	
	retval = mtd->block_isbad(mtd,blockNo* dev->nChunksPerBlock * dev->nBytesPerChunk);
	
	if(retval)
	{
		T(YAFFS_TRACE_MTD,(TSTR("block is bad" TENDSTR)));
	
		*state = YAFFS_BLOCK_STATE_DEAD;
		*sequenceNumber = 0;
	}
	else
	{
		yaffs_ExtendedTags t;
		nandmtd2_ReadChunkWithTagsFromNAND(dev,blockNo * dev->nChunksPerBlock,NULL, &t);
		
		if(t.chunkUsed)
		{
		  *sequenceNumber = t.sequenceNumber;
		  *state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
		}
		else
		{
		  *sequenceNumber = 0;
 		  *state = YAFFS_BLOCK_STATE_EMPTY;
		}
	}
		T(YAFFS_TRACE_MTD,(TSTR("block is bad seq %d state %d" TENDSTR), *sequenceNumber,*state));

    if (retval == 0)
    	return YAFFS_OK;
    else
        return YAFFS_FAIL;
}

#endif



