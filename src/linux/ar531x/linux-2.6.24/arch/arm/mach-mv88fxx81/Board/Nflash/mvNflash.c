/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
#include "mvOs.h"
#include "mvNflash.h"

#undef MV_DEBUG

#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

#define X8_INV_BLK_SIZE		1
#define X16_INV_BLK_SIZE	2

#if 0
/* This structure describes x8 and x16 unified data astructure that 		*/
/* describes the spare area. The spare area of each page keeps vital data	*/
/* used for block replacement algorithm and ECC such as invalid block, ECC 	*/
/* and next block offset which is saved in case of power failure.           */
typedef struct _nflashSpare
{
    MV_U8	x16InvBlk1[X16_INV_BLK_SIZE]; /* x16 1st Invalid block indicator*/
    MV_U8	plainAEcc[ECC_SIZE];		  /* Plain A ECC array          	*/
	MV_U8	pad1;						  /* Padding						*/
    MV_U8	x8invBlk[X8_INV_BLK_SIZE];	  /* x8 Invalid block indicator 	*/
    MV_U8	plainBEcc[ECC_SIZE];		  /* Plain A ECC array          	*/
	MV_U8	pad2;						  /* Padding						*/
    MV_U8	x16InvBlk2[X16_INV_BLK_SIZE]; /* x16 2nd Invalid block indicator*/
	MV_U16	nextBlkOffs					  /* Next valid block offset		*/
}NFLASH_SPARE;
#endif

#define LAST_FLASH 0xFFFFFFFF

/* Note that this table size parameter is in bytes for x8 devie and in  */
/* words for x16 devices.                                               */    
NFLASH_STRUCT supNflashAry[]=
{
	/*  flashVen     flashId     size  #blocks pageDsize spareSize #pagePerBlk*/
	{SAMSUNG_MANUF, K9F5608Q0C,  _32M,  _2K,     512,      16,         32},
	{SAMSUNG_MANUF, K9F5608D0C,  _32M,  _2K,     512,      16,         32},
	{SAMSUNG_MANUF, K9F5608U0C,  _32M,  _2K,     512,      16,         32},
	{SAMSUNG_MANUF, K9F5616Q0C,  _32M,  _2K,     256,       8,         32},
	{SAMSUNG_MANUF, K9F5616D0C,  _32M,  _2K,     256,       8,         32},
	{SAMSUNG_MANUF, K9F5616U0C,  _32M,  _2K,     256,       8,         32},
	{LAST_FLASH,    LAST_FLASH,     0,    0,       0,       0,          0}
};


/* Local routines */
static MV_STATUS prgmEraseStsGet(MV_NFLASH_INFO *pFlash);
static MV_U32 readData(MV_NFLASH_INFO *pFlash, MV_U32 readType, MV_U32 offset, MV_U32 size, MV_U8 *pData);

/*******************************************************************************
* mvNflashInit - Initialize the Flash indetifier structure.
*
* DESCRIPTION:
*		This function initialize the Flash indetifier structure. 
*		Note that this function must be called only after the MV_NFLASH_HW_IF
*		field in the MV_NFLASH_INFO was initialized by the BSP. Only then 
*		access to nflash is possible.
*
* INPUT:
*		pFlash - flash information.
*
* OUTPUT:
*		Initialize MV_NFLASH_INFO Nflash identifier struct.
*		
*
* RETURN:
*		MV_ERROR if device and vendor ID can't be read.
*		MV_OK otherwise.
*
*******************************************************************************/
MV_STATUS mvNflashInit(MV_NFLASH_INFO *pFlash)
{
	MV_U16 flashId;
	int i = 0;
    
	if(NULL == pFlash)
		return 0;

	/* Initialize base address and device width from BSP */
	pFlash->baseAddr = pFlash->nflashHwIf.devBaseAddr;
	pFlash->devWidth = pFlash->nflashHwIf.devWidth;
		
    /* Initialize HW interface */
    mvNflashHwIfInit(&pFlash->nflashHwIf);

	/* Read the device and vendor ID */
	flashId = mvNflashIdGet(pFlash);

	/* check if this flash is Supported, and Init the pFlash flash feild */	
    while((supNflashAry[i].devVen != ((flashId << VENDOR_ID_OFFS) & 0xFF) && 
		  (supNflashAry[i].devId  != ((flashId << DEVICE_ID_OFFS) & 0xFF))))
	{		
		if (i++ == (sizeof(supNflashAry) / sizeof(supNflashAry[0])))
		{
			mvOsPrintf("mvNflashInit: Unsupported Flash ID 0x%x.\n", flashId);
			return MV_ERROR;
		}
	}

	pFlash->pNflashStruct = &supNflashAry[i];

	/* Init pFlash sectors */	
	/*
	if(MV_OK != flashSecsInit(pFlash))
	{
		mvOsPrintf("Flash: ERROR mvFlashInit flashSecsInit failed \n");
		return 0;
	}*/
	
    /* print all flash information */
	DB(mvNflashPrint(pFlash));

	/* reset the Flash */
	mvNflashReset(pFlash);

	return MV_OK;
	}

/*******************************************************************************
* mvNflashReset - Reset the Flash.
*
* DESCRIPTION:
*	This function activates the Flash reset command.
*
* INPUT:
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	None
*
*******************************************************************************/
MV_VOID mvNflashReset(MV_NFLASH_INFO *pFlash)
{
	mvNflashCommandSet(&pFlash->nflashHwIf, RESET_FLASH);
}

/*******************************************************************************
* mvNflashIdGet - Get Flash device and vendor ID.
*
* DESCRIPTION:
*	This function gets Flash device and vendor ID.
*
* INPUT:
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	None
*
*******************************************************************************/
MV_U16 mvNflashIdGet(MV_NFLASH_INFO *pFlash)
{
	MV_U8 venId;
	MV_U8 devId;
	
	mvNflashCommandSet(&pFlash->nflashHwIf, READ_ID);
	mvNflashAddrSet(&pFlash->nflashHwIf, 0x00);

	/* Wait for Tr. */
	mvOsUDelay(NFLASH_tAR_DELAY);

	mvNflash8bitDataGet(&pFlash->nflashHwIf, (void*)&venId, 1);

	mvNflash8bitDataGet(&pFlash->nflashHwIf, (void*)&devId, 1);

	return ((venId << VENDOR_ID_OFFS) | (devId << DEVICE_ID_OFFS)) & 0xFFFF;
}


/*******************************************************************************
* mvNflashPageProg - Program flash page.
*
* DESCRIPTION:
*	    This function programs up to one page. It could program partial page
*       as well.
*
* INPUT:
*       pFlash - Flash information.
*       offset - Page offset in bytes.
*       size   - Data size in bytes for x8 and in words for x16 devices.
*       pData  - Buffer to copy data.
*
* OUTPUT:
*       None
*
* RETURN:
*       MV_TIMEOUT in case the device is not in ready state within timeout.
*	    MV_OK if the operation succeeded
*       MV_FAIL if the operation failed.
*       MV_WRITE_PROTECT if the operation failed because of write protection.
*
*******************************************************************************/
MV_STATUS mvNflashPageProg(MV_NFLASH_INFO *pFlash, MV_U32 offset, MV_U32 size, MV_U8 *pData)
{
    MV_U32 tmpSize;
    
    /* Set first program command */
    mvNflashCommandSet(&pFlash->nflashHwIf, PAGE_PROGRAM_CMD1);

    /* Set address: 1) Column address */
    mvNflashAddrSet(&pFlash->nflashHwIf, (offset         & 0xFF));
    /* Set address: 2.1) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((offset >>  9) & 0xFF));
    /* Set address: 2.2) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((offset >> 17) & 0xFF));
    

    /* 1st phase. Calculate first page data left size   */
    tmpSize = pFlash->pNflashStruct->pageDataSize + 
              pFlash->pNflashStruct->spareSize    - 
              (offset & 0xFF);
    
    /* We can copy maximum page Size Left, but actual size can be smaller     */
    tmpSize = MV_MIN(tmpSize, size);    

    /* Write the data to Flash register */
    (*pFlash->nflashHwIf.nfDataSetRtn) (&pFlash->nflashHwIf, (void*)pData, tmpSize);

    /* Set second program command */
    mvNflashCommandSet(&pFlash->nflashHwIf, PAGE_PROGRAM_CMD2);

    return(prgmEraseStsGet(pFlash));
}


/*******************************************************************************
* mvNflashCpBackProg - Cpoy one block to another erased one.
*
* DESCRIPTION:
*       The copy-back program is configured to quickly and efficiently 
*       rewrite data stored in one page within the array to another page 
*       within the same array without utilizing an external memory.
*       Although the command enables to perform copyback from any page offset, 
*       this function copies the whole page only.
*
* INPUT:
*       pFlash  - Flash information.
*       srcPage - Source Page offset.
*       dstPage - Destination Page offset.
*
* OUTPUT:
*       None
*
* RETURN:
*       MV_TIMEOUT in case the device is not in ready state within timeout.
*	    MV_OK if the operation succeeded
*       MV_FAIL if the operation failed.
*       MV_WRITE_PROTECT if the operation failed because of write protection.
*
*******************************************************************************/
MV_STATUS mvNflashCpBackProg(MV_NFLASH_INFO *pFlash, MV_U32 srcOffs, MV_U32 dstOffs)
{
    /* Set Read command */
    mvNflashCommandSet(&pFlash->nflashHwIf, COPY_BACK_PGR_CMD1);
        
    /* Set address: 1) Column address */
    mvNflashAddrSet(&pFlash->nflashHwIf, 0);
    /* Set address: 2.1) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((srcOffs >>  9) & 0xFF));
    /* Set address: 2.2) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((srcOffs >> 17) & 0xFF));

    /* Wait for device to be ready */
    if (MV_TIMEOUT == prgmEraseStsGet(pFlash))
        return MV_TIMEOUT;
    
    /* Set copyback command */
    mvNflashCommandSet(&pFlash->nflashHwIf, COPY_BACK_PGR_CMD2);
        
    /* Set address: 1) Column address */
    mvNflashAddrSet(&pFlash->nflashHwIf, 0);
    /* Set address: 2.1) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((dstOffs >>  9) & 0xFF));
    /* Set address: 2.2) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((dstOffs >> 17) & 0xFF));

    return(prgmEraseStsGet(pFlash));
}     
/*******************************************************************************
* mvNflashPageSpareRead - Read page spare area.
*
* DESCRIPTION:
*	    This function reads data of given size from a given page starting 
*       from given offset in the spair area.
*
* INPUT:
*       pFlash    - Flash information.
*       pageOffs  - Page offset in bytes.
*       spairOffs - spare offset in bytes for x8 and in words for x16 devices.
*       pData     - Buffer to copy data.
*       size      - Data size in bytes for x8 and in words for x16 devices.
*
* OUTPUT:
*       None
*
* RETURN:
*       Size of copied data.
*
*******************************************************************************/
MV_U32 mvNflashPageSpareRead(MV_NFLASH_INFO *pFlash, 
                             MV_U32 pageOffs, 
                             MV_U32 spairOffs, 
                             MV_U32 size,
                             MV_U8 *pData)
{    
    MV_U8 offsMask;

    
    if (NFLASH_DEV_WIDTH_x8 == pFlash->nflashHwIf.devWidth)
    {
        if (spairOffs > 15) 
        {
            mvOsPrintf("NAND Flash Spare read: Err Wrong offset %d\n", spairOffs);
            return 0;
        }

        offsMask = 0xF; /* X8 device : A4 ~ A7 Don't care */
    }
    else
    {
        if (spairOffs > 8) 
        {
            mvOsPrintf("NAND Flash Spare read: Err Wrong offset %d\n", spairOffs);
            return 0;
        }

        offsMask = 0x7; /* X16 device : A3 ~ A7 are "L" */        
    }

    /* Set Read 2 command */
    mvNflashCommandSet(&pFlash->nflashHwIf, READ_SPARE);
        
    /* Set address: 1) Column address */
    mvNflashAddrSet(&pFlash->nflashHwIf, (spairOffs  & offsMask));
    /* Set address: 2.1) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((pageOffs >>  9) & 0xFF));
    /* Set address: 2.2) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((pageOffs >> 17) & 0xFF));

    /* Wait for device to be ready */
    if (MV_TIMEOUT == prgmEraseStsGet(pFlash))
        return 0;
    
    /* Set Read command */
    mvNflashCommandSet(&pFlash->nflashHwIf, READ_SPARE);
    
    return (readData(pFlash, READ_SPARE, spairOffs, size, pData));
}

/*******************************************************************************
* mvNflashRead - Read Flash data.
*
* DESCRIPTION:
*	    This function reads data of given size starting from given page offset.
*       This data also includes the spair area.
*       Note: In 16-bit devices the offsets are in words (16-bit) which means 
*       data size and base must be aligned to 2.
*       
*
* INPUT:
*       pFlash - Flash information.
*       offset - Flash data offset in bytes.
*       pData  - Buffer to copy data.
*       size   - Data size in bytes for x8 and in words for x16 devices.
*
* OUTPUT:
*       None
*
* RETURN:
*       Size of copied data.
*
*******************************************************************************/
MV_U32 mvNflashPageRead(MV_NFLASH_INFO *pFlash, MV_U32 offset, MV_U32 size, MV_U8 *pData)
{    
    MV_U32 readType;
    
    /* If device is 8-bit and offset is in plain B, active the correct cmd  */
    if ((pFlash->devWidth == NFLASH_DEV_WIDTH_x8) &&
        ((offset % pFlash->pNflashStruct->pageDataSize) > NFLASH_DATA_PLAIN_SIZE))
    {
        /* Set Read command for plain B */
        readType = READ_B_PLAIN;
    }
    else
    {
        /* Set Read command for plain A */
        readType = READ_A_PLAIN;
    }
    
    /* Set Read command for plain B */
    mvNflashCommandSet(&pFlash->nflashHwIf, readType);
        
    /* Set address: 1) Column address */
    mvNflashAddrSet(&pFlash->nflashHwIf, (offset         & 0xFF));
    /* Set address: 2.1) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((offset >>  9) & 0xFF));
    /* Set address: 2.2) Row (page) address */
    mvNflashAddrSet(&pFlash->nflashHwIf, ((offset >> 17) & 0xFF));

    /* Wait for device to be ready */
    if (MV_TIMEOUT == prgmEraseStsGet(pFlash))
        return 0;
    
    /* Set Read command */
    mvNflashCommandSet(&pFlash->nflashHwIf, readType);

    return (readData(pFlash, readType, offset, size, pData));
}

/*******************************************************************************
* readData - Read the data from flash port into user buffer.
*
* DESCRIPTION:
*	    This function Read the data from flash port into user buffer.
*       The data is ready in the flash buffers. This function supports the
*       K9F5608D0C and K9F5608U0C devices sequential page reads.
*       Note that the offset parameter does not concerns the page spare section.
*       Nevertheless, the size does include the spare section. This is why
*       we use maxSize to describe the total page size that can be read 
*       (528 bytes) and maxDataSize to describe the total data in page that 
*       can be read (512). When using read 2 operation this is not the case 
*       as both total size and data size are the same (16 bytes)
*
* INPUT:
*       pFlash   - Flash information.
*       readType - Read operation type 1 (page) or 2 (spare area)
*       offset   - Flash data offset in bytes.
*       pData    - Buffer to copy data.
*       size     - Data size in bytes for x8 and in words for x16 devices.
*
* OUTPUT:
*       None
*
* RETURN:
*       Size of copied data.
*
*******************************************************************************/
static MV_U32 readData(MV_NFLASH_INFO *pFlash, MV_U32 readType, MV_U32 offset, MV_U32 size, MV_U8 *pData)
{
    int i;
    MV_U32 tmpSize;
    MV_U32 maxSize;        /* total page size that can be read      */
    MV_U32 maxDataSize;    /* total data in page that can be read   */
    MV_U32 numOfFullPages; /* Number of full pages to read */

    if (READ_SPARE == readType)
    {
        maxDataSize = pFlash->pNflashStruct->spareSize;
        maxSize = maxDataSize;
    }
    else
    {
        maxDataSize = pFlash->pNflashStruct->pageDataSize;
        maxSize = maxDataSize + pFlash->pNflashStruct->spareSize;
    }
    
    /* Start copy data. Copying data is done on page boundry, thus copying  */
    /* is done in three phases.                                             */
    /* 1) Copy data from first page.                                        */
    /* 2) If the size overflow to the next pages copy full pages data       */
    /* 3) Copy the last page data.                                          */

    /* 1st phase. Calculate first page data left size   */
    tmpSize = maxSize - (offset & 0xFF);
    
    /* We can copy maximum pageSizeLeft, but actual size can be smaller     */
    tmpSize = MV_MIN(tmpSize, size);    

    /* Copy data from first page */
    (*pFlash->nflashHwIf.nfDataGetRtn) (&pFlash->nflashHwIf, (void*)pData, tmpSize);
    
    /* Size is limited to page baundry. For K9F5608D0C and K9F5608U0C   */
    /* size is unlimited (sequential page read)                         */
    if (K9F5608D0C != pFlash->pNflashStruct->devId)
    {
        return tmpSize;
    }

    /* must wait at the end of page read */
    if (MV_TIMEOUT == prgmEraseStsGet(pFlash))
        return tmpSize;
    
    /* Set Read command */
    mvNflashCommandSet(&pFlash->nflashHwIf, readType);

    /* Move forward the data pointer */
    pData += tmpSize;

    /* calculate the size left after the first page */
    tmpSize = size - tmpSize;

    /* Calculate number of full pages to read */
    numOfFullPages = tmpSize / maxSize;

    /* 2nd pase. If the size overflow to the next pages copy full pages data */
    for (i = 0; i < numOfFullPages; i++, pData += maxSize)
    {
        (*pFlash->nflashHwIf.nfDataGetRtn) 
            (&pFlash->nflashHwIf, (void*)pData, maxSize);        

        tmpSize-= maxSize;
        
        /* Wait for device to be ready for next loop */
        if (MV_TIMEOUT == prgmEraseStsGet(pFlash))
            return tmpSize;

        /* Set Read command */
        mvNflashCommandSet(&pFlash->nflashHwIf, readType);
    }
    
    /* No third phase. tmpSize was aligned to page size. */
    if (0 == tmpSize)
    {
        return size;
    }

    /* 3rd phase. Copy the last page data   */
    (*pFlash->nflashHwIf.nfDataGetRtn) (&pFlash->nflashHwIf, (void*)pData, tmpSize);

    /* must wait at the end of page read */
    if (MV_TIMEOUT == prgmEraseStsGet(pFlash))
        return tmpSize;
    
    return size;
    
}

/*******************************************************************************
* mvNflashBlockErase - Erase flash block.
*
* DESCRIPTION:
*	    This function erases flash block.
*
* INPUT:
*       pFlash - Flash information.
*       blkNum - Block number
*
* OUTPUT:
*       None
*
* RETURN:
*       MV_TIMEOUT in case the device is not in ready state within timeout.
*	    MV_OK if the operation succeeded
*       MV_FAIL if the operation failed.
*       MV_WRITE_PROTECT if the operation failed because of write protection.
*
*******************************************************************************/
MV_STATUS mvNflashBlockErase(MV_NFLASH_INFO *pFlash, MV_U32 blkNum)
{
    MV_U32 blkOffs;

    blkOffs = mvNflashBlkOffsGet(pFlash, blkNum);
    
    /* Command phase 1 */
    mvNflashCommandSet(&pFlash->nflashHwIf, BLOCK_ERASE_CMD1);

    mvNflashAddrSet(&pFlash->nflashHwIf, ((blkOffs >> 9)  & 0xFF));
    mvNflashAddrSet(&pFlash->nflashHwIf, ((blkOffs >> 17) & 0xFF));

    /* Command phase 2 */
    mvNflashCommandSet(&pFlash->nflashHwIf, BLOCK_ERASE_CMD2);

    return(prgmEraseStsGet(pFlash));
}

/*******************************************************************************
* mvNflashBlockLock - Lock the entire flash blocks.
*
* DESCRIPTION:
*	    This function locks the entire flash blocks.
*
* INPUT:
*       pFlash - Flash information.
*
* OUTPUT:
*       None
*
* RETURN:
*	    None.
*
* NOTE:
*       Block Lock mode is enabled while LOCKPRE pin state is high, 
*       which is to offer protection features for NAND Flash data.
*
*******************************************************************************/
MV_VOID mvNflashBlockLock(MV_NFLASH_INFO *pFlash)
{
    mvNflashCommandSet(&pFlash->nflashHwIf, FLASH_LOCK);
}

/*******************************************************************************
* mvNflashBlockLockTight - Lock Tight the entire flash blocks.
*
* DESCRIPTION:
*	    This function locks tight the entire flash blocks.
*
* INPUT:
*       pFlash - Flash information.
*
* OUTPUT:
*       None
*
* RETURN:
*	    None.
*
* NOTE:
*       Block Lock mode is enabled while LOCKPRE pin state is high, 
*       which is to offer protection features for NAND Flash data.
*
*******************************************************************************/
MV_VOID mvNflashBlockLockTight(MV_NFLASH_INFO *pFlash)
{
    mvNflashCommandSet(&pFlash->nflashHwIf, FLASH_LOCK_TIGHT);
}

/*******************************************************************************
* mvNflashBlockUnlock - Lock Tight the entire flash blocks.
*
* DESCRIPTION:
*	    This function unlocks a range of blocks.
*
* INPUT:
*       pFlash      - Flash information.
*       startBlkNum - Starting from blk number.
*       endBlkNum   - Ending at blk number.
*
* OUTPUT:
*       None
*
* RETURN:
*	    None.
*
* NOTE:
*       Block Lock mode is enabled while LOCKPRE pin state is high, 
*       which is to offer protection features for NAND Flash data.
*
*******************************************************************************/
MV_VOID mvNflashBlockUnlock(MV_NFLASH_INFO *pFlash, MV_U32 startBlkNum, MV_U32 endBlkNum)
{    
    MV_U32 blkOffs;
    MV_U8   dummy = 0;

    blkOffs = mvNflashBlkOffsGet(pFlash, startBlkNum);
    
    /* Command phase 1 */
    mvNflashCommandSet(&pFlash->nflashHwIf, BLOCK_UNLOCK_CMD1);

    mvNflashAddrSet(&pFlash->nflashHwIf, ((blkOffs >> 9)  & 0xFF));
    mvNflashAddrSet(&pFlash->nflashHwIf, ((blkOffs >> 17) & 0xFF));

    blkOffs = mvNflashBlkOffsGet(pFlash, endBlkNum);

    /* Command phase 2 */
    mvNflashCommandSet(&pFlash->nflashHwIf, BLOCK_UNLOCK_CMD2);

    mvNflashAddrSet(&pFlash->nflashHwIf, ((blkOffs >> 9)  & 0xFF));
    mvNflashAddrSet(&pFlash->nflashHwIf, ((blkOffs >> 17) & 0xFF));

    /* According to the spec we need another cycle with WE only */
    mvNflash8bitDataSet (&pFlash->nflashHwIf, &dummy, 1);
}

/*******************************************************************************
* mvNflashBlockLockStsGet - Get the block lock status.
*
* DESCRIPTION:
*	    This function gets the block lock status.
*
* INPUT:
*       pFlash - Flash information.
*       offset - Offset (in bytes) inside block in question.
*
* OUTPUT:
*       None
*
* RETURN:
*	    Block lock status.
*
*******************************************************************************/
NFLASH_LOCK_MODE mvNflashBlockLockStsGet(MV_NFLASH_INFO *pFlash,MV_U32 blkNum)
{
    MV_U32 blkOffs;
    MV_U8 lockReg;

    blkOffs = mvNflashBlkOffsGet(pFlash, blkNum);
    
    mvNflashCommandSet(&pFlash->nflashHwIf, READ_BLOCK_LOCK_STS);
    
    mvNflashAddrSet(&pFlash->nflashHwIf, ((blkOffs >> 9)  & 0xFF));
    mvNflashAddrSet(&pFlash->nflashHwIf, ((blkOffs >> 17) & 0xFF));

    /* Wait for device to be ready */
    if (MV_TIMEOUT == prgmEraseStsGet(pFlash))
        return 0;
	
    /* Write the read lock status command again */
    mvNflashCommandSet(&pFlash->nflashHwIf, READ_BLOCK_LOCK_STS);

	mvNflash8bitDataGet(&pFlash->nflashHwIf, (void*)&lockReg, 1);

    if (lockReg & BLSR_UNLOCK_BIT)
    {
        return BLOCK_UNLOCK;
    }
    else if (lockReg & BLSR_LOCK_BIT)
    {
        return BLOCK_LOCK;
    }
    else if (lockReg & BLSR_LOCK_TIGHT_BIT)
    {
        return BLOCK_TIGHTLOCK;
    }
    else
        return BLOCK_UNDEFINED_LOCK;
}

/*******************************************************************************
* prgmEraseStsGet - Report valid status of programm/erase operations.
*
* DESCRIPTION:
*	    This function reports valid status of programm/erase operations.
*       This function polls on I/O 6 bit of the read status register for the
*       busy/ready indication. Only after the register is in ready state the
*       function returns the I/O 0 bit status ('0' -> OK, '1' -> Fail).
*
* INPUT:
*       pFlash	- Flash information.
*
* OUTPUT:
*       None
*
* RETURN:
*       MV_TIMEOUT in case the device is not in ready state 
*        for RSR_PROGRAM_ERASE_STAT_MASK attempts to read the status register.
*	    MV_OK if I/O 0 is reset ('0') and I/0 bit 7 is set ('1')
*       MV_FAIL if I/O 0 bit is set ('1')
*       MV_WRITE_PROTECT if I/O bit 7 is reset ('0')
*
*******************************************************************************/
static MV_STATUS prgmEraseStsGet(MV_NFLASH_INFO *pFlash)
{
    MV_U8 readStsReg;
    MV_U32 timeout = NFLASH_PROG_ERASE_TIMEOUT;

    /* Set read starus register command */
    mvNflashCommandSet(&pFlash->nflashHwIf, READ_STATUS);
    
    /* Wait for ready status I/O 6 to be '1' */
    while(timeout--)
    {
        /* Read status register value */
        mvNflash8bitDataGet(&pFlash->nflashHwIf, (void*)&readStsReg, 1);
        
        /* Chaeck if Flash is ready */
        if ((readStsReg & RSR_DEV_OPERATION_MASK) == RSR_DEV_OPERATION_READY) 
        {
            /* Check for write protect status */ 
            if ((readStsReg & RSR_WRITE_PROTECT_MASK) == RSR_WRITE_PROTECTED)
            {
                return MV_WRITE_PROTECT;
            }
            
            /* Check for program/erase operation status */
            if ((readStsReg & RSR_PROG_ERASE_MASK) == RSR_PROG_ERASE_SUCCESS)
            {
                return MV_OK;
            }
            else
            {
                return MV_FAIL;
            }
        }
    }
    
    return MV_TIMEOUT;
}

/*******************************************************************************
* mvNflashReadStsRegGet - Get the read status register value.
*
* DESCRIPTION:
*	    This function gets the read status.
*
* INPUT:
*       pFlash	- Flash information.
*
* OUTPUT:
*       None
*
* RETURN:
*	    Read status mask (as in the register).
*
*******************************************************************************/
MV_U32 mvNflashReadStsRegGet(MV_NFLASH_INFO *pFlash)
{
    MV_U8 readStsReg;

    mvNflashCommandSet(&pFlash->nflashHwIf, READ_STATUS);
    mvNflash8bitDataGet(&pFlash->nflashHwIf, (void*)&readStsReg, 1);

    return readStsReg;
}

/*******************************************************************************
* mvNflashBlkOffsGet - Get a block number offset in Flash.
*
* DESCRIPTION:
*	    This function get a block number offset in Flash.
*
* INPUT:
*       pFlash - Flash information.
*       blkNum - Block number
*
* OUTPUT:
*       None
*
* RETURN:
*	    Offset of the block in the Flash.
*
*******************************************************************************/
MV_U32 mvNflashBlkOffsGet(MV_NFLASH_INFO *pFlash, MV_U32 blkNum)
{
    return ((pFlash->pNflashStruct->size) / 
            (pFlash->pNflashStruct->blockNum)) * blkNum;
}

/*******************************************************************************
* mvNflashPrint - Print NAND flash information structure.
*
* DESCRIPTION:
*	Prints all the feilds in the NAND flash info structure.
*
* INPUT:
*       pFlash	- Flash information.
*
* OUTPUT:
*       None
*
* RETURN:
*	None
*
*******************************************************************************/
MV_VOID mvNflashPrint(MV_NFLASH_INFO *pFlash)
{
	int i, lockedBlocksFound = 0;
    NFLASH_LOCK_MODE lockMode;
	
    if (NULL == pFlash)
	{
		mvOsOutput ("Missing or unknown FLASH type\n");
		return;
	}

    mvOsOutput ("NAND Flash information:\n");

    mvOsOutput ("Manufacture: ");
	switch (pFlash->pNflashStruct->devVen)
	{
		case SAMSUNG_MANUF:     	
			mvOsOutput ("SAMSUNG\n");		
			break;
		default:                
			mvOsOutput ("Unknown Vendor 0x%x\n", pFlash->pNflashStruct->devVen);	
			break;
    }

    mvOsOutput ("Flash Type : ");
	switch (pFlash->pNflashStruct->devId)
	{
		case K9F5608Q0C:
			mvOsOutput ("K9F5608Q0C (x8-bit)\n");
			break;
		case K9F5608D0C:
			mvOsOutput ("K9F5608D0C or K9F5608U0C(x8-bit)\n");
			break;
		case K9F5616Q0C:
			mvOsOutput ("K9F5616Q0C (x16-bit)\n");
			break;
		case K9F5616D0C:
			mvOsOutput ("K9F5616D0C or K9F5616U0C(x16-bit)\n");
			break;
		default:
			mvOsOutput ("Unknown device id 0x%x\n", pFlash->pNflashStruct->devId);
			break;
	}
	
	mvOsOutput("Flash ");
	mvSizePrint(pFlash->pNflashStruct->size);
    mvOsOutput ("\n");
	mvOsOutput("Flash base Address: 0x%x\n",pFlash->baseAddr);
	mvOsOutput("Flash device Width: %d.\n", pFlash->devWidth);
	mvOsOutput("Number of blocks: %d\n", pFlash->pNflashStruct->blockNum);


	mvOsOutput ("Block Start Addresses:");
	for (i = 0; i < pFlash->pNflashStruct->blockNum; ++i) 
    {
		lockMode = mvNflashBlockLockStsGet(pFlash,i);

        if (BLOCK_UNLOCK == lockMode)
        {
            continue;
        }
        
        lockedBlocksFound++;

        if ((i % 5) == 0)
			mvOsOutput ("\n   ");
	    
        mvOsOutput (" %08x %s", mvNflashBlkOffsGet(pFlash, i), 
                    (lockMode == BLOCK_LOCK) ? "Lock" : "Tight");
	    
    }
	mvOsOutput("\n");

    mvOsOutput("Found %d Read only blocks\n", lockedBlocksFound);

	return;
}
