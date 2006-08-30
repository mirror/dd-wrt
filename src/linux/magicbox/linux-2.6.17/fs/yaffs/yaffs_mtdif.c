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

const char *yaffs_mtdif_c_version = "$Id: yaffs_mtdif.c,v 1.7 2005/08/01 20:52:35 luc Exp $";
 
#include "yportenv.h"

#ifdef CONFIG_YAFFS_YAFFS1

#include "yaffs_mtdif.h"

#include "linux/mtd/mtd.h"
#include "linux/types.h"
#include "linux/time.h"
#include "linux/mtd/nand.h"

static struct nand_oobinfo yaffs_oobinfo = {
	.useecc = 1,
	.eccbytes = 6,
	.eccpos = {8, 9, 10, 13, 14, 15}
};

static struct nand_oobinfo yaffs_noeccinfo = {
	.useecc = 0,
};


int nandmtd_WriteChunkToNAND(yaffs_Device *dev,int chunkInNAND,const __u8 *data, const yaffs_Spare *spare)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	size_t dummy;
    int retval = 0;
	
	loff_t addr = ((loff_t)chunkInNAND) * dev->nBytesPerChunk;
	
	__u8 *spareAsBytes = (__u8 *)spare;

	if(data && spare)
	{
		if(dev->useNANDECC)
			retval = mtd->write_ecc(mtd,addr,dev->nBytesPerChunk,&dummy,data,spareAsBytes,&yaffs_oobinfo);
		else
			retval = mtd->write_ecc(mtd,addr,dev->nBytesPerChunk,&dummy,data,spareAsBytes,&yaffs_noeccinfo);
	}
	else
	{
	if(data)
		retval = mtd->write(mtd,addr,dev->nBytesPerChunk,&dummy,data);
	if(spare)
		retval = mtd->write_oob(mtd,addr,YAFFS_BYTES_PER_SPARE,&dummy,spareAsBytes);
	}

    if (retval == 0)
    	return YAFFS_OK;
    else
        return YAFFS_FAIL;
}

int nandmtd_ReadChunkFromNAND(yaffs_Device *dev,int chunkInNAND, __u8 *data, yaffs_Spare *spare)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	size_t dummy;
    int retval = 0;
	
	loff_t addr = ((loff_t)chunkInNAND) * dev->nBytesPerChunk;
	
	__u8 *spareAsBytes = (__u8 *)spare;
	
	if(data && spare)
	{
		if(dev->useNANDECC)
		{   // Careful, this call adds 2 ints to the end of the spare data.  Calling function should
            // allocate enough memory for spare, i.e. [YAFFS_BYTES_PER_SPARE+2*sizeof(int)].
    		retval = mtd->read_ecc(mtd,addr,dev->nBytesPerChunk,&dummy,data,spareAsBytes,&yaffs_oobinfo);            
		}
		else
		{
			retval = mtd->read_ecc(mtd,addr,dev->nBytesPerChunk,&dummy,data,spareAsBytes,&yaffs_noeccinfo);
		}
	}
	else
	{
	if(data)
		retval = mtd->read(mtd,addr,dev->nBytesPerChunk,&dummy,data);
	if(spare)
		retval = mtd->read_oob(mtd,addr,YAFFS_BYTES_PER_SPARE,&dummy,spareAsBytes);
	}

    if (retval == 0)
    	return YAFFS_OK;
    else
        return YAFFS_FAIL;
}

// Callback not needed for NAND
#if 0
static void nandmtd_EraseCallback(struct erase_info *ei)
{
	yaffs_Device *dev = (yaffs_Device *)ei->priv;	
	up(&dev->sem);
}
#endif


int nandmtd_EraseBlockInNAND(yaffs_Device *dev, int blockNumber)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	__u32 addr = ((loff_t) blockNumber) * dev->nBytesPerChunk * dev->nChunksPerBlock;
	struct erase_info ei;
    int retval = 0;
	
	ei.mtd = mtd;
	ei.addr = addr;
	ei.len = dev->nBytesPerChunk * dev->nChunksPerBlock;
	ei.time = 1000;
	ei.retries = 2;
	ei.callback = NULL;
	ei.priv = (u_long)dev;
	
	// Todo finish off the ei if required
	
	sema_init(&dev->sem,0);

	retval = mtd->erase(mtd,&ei);	
	
	//No need for callback 
	// down(&dev->sem); // Wait for the erasure to complete

    if (retval == 0)	
    	return YAFFS_OK;
    else
        return YAFFS_FAIL;
}

int nandmtd_InitialiseNAND(yaffs_Device *dev)
{
	return YAFFS_OK;
}

#endif // CONFIG_YAFFS_YAFFS1

