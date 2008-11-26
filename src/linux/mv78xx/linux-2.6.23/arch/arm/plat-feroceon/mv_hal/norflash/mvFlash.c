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

#include "mvFlash.h"


#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

#define LAST_FLASH 0xFFFFFFFF
/* When inserting a bottom/top flash number of sectors should be the number */
/* of all sector including the "fragmented" sectors.                        */
/*                                                                          */
/* Size of the flash must be as spesified in Spec, in case there are 2 dev  */
/* in a row the driver will recognize it according to bus and dev width.    */
/*                                                                          */
/* In case of using Bottom flash it should look like this:                  */
/* MV_U32 IntelSecFrag[]= {_8K, _8K, _8K, _8K, _8K, _8K, _8K, _8K};         */
/* FLASH_STRUCT supFlashAry[]=                                              */
/* {                                                                        */
/*  {                                                                       */
/*   INTEL_MANUF,           flash Vendor                                    */
/*   INTEL_FID_28F640J3A,   flash ID                                        */
/*   _4M,                   flash size                                      */
/*   71,                    flash secotor number                            */
/*   BOTTOM,                flash sector structure (top, bottom, regular)   */
/*   8,                     Number of sector fragments                      */
/*   IntelSecFrag,          pointer to sector size fragment list            */
/*   MV_FALSE               Support of HW protection                        */
/*  },                                                                      */
/*   .......                                                                */
/* }                                                                        */
/*                                                                          */
FLASH_STRUCT supFlashAry[]=
{
/*  flashVen       flashId         size  #sec  secType #Frag pFragList HWprot HwWrBuff */
{INTEL_MANUF, INTEL_FID_28F640J3A, _8M,   64,  REGULAR,  0,   NULL, MV_TRUE,  32 },
{INTEL_MANUF, INTEL_FID_28F128J3A, _16M,  128, REGULAR,  0,   NULL, MV_TRUE,  32 },
{SST_MANUF,   SST_39VF_020,        _256K, 64,  REGULAR,  0,   NULL, MV_FALSE,  0 },
{AMD_MANUF,   AMD_FID_LV040B,      _512K, 8,   REGULAR,  0,   NULL, MV_FALSE,  0 },
{STM_MANUF,   STM_FID_29W040B,     _512K, 8,   REGULAR,  0,   NULL, MV_FALSE,  0 },
{AMD_MANUF,   AMD_S29GL128N,	   _16M,  128, REGULAR,  0,   NULL, MV_FALSE},
{MX_MANUF,    AMD_FID_MIRROR,      _16M,  256, REGULAR,  0,   NULL, MV_FALSE},
{STM_MANUF,   AMD_FID_MIRROR,      _16M,  256, REGULAR,  0,   NULL, MV_FALSE},
{LAST_FLASH,  LAST_FLASH,          0,     0,   REGULAR,  0,   NULL, MV_FALSE,  0 }
};

static MV_STATUS flashReset(MV_FLASH_INFO *pFlash);
static MV_STATUS flashStructGet(MV_FLASH_INFO *pFlash, MV_U32 manu, MV_U32 id);
static MV_STATUS flashSecsInit(MV_FLASH_INFO *pFlash);
static MV_BOOL flashSecLockGet(MV_FLASH_INFO *pBlock,MV_U32 secNum);
static MV_STATUS mvFlashProg(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U32 data);
static MV_U32 flashGetHwBuffSize(MV_FLASH_INFO *pFlash);
static MV_STATUS flashHwBufferProg(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U32 byteCount,
				   MV_U8 *pData);

/*******************************************************************************
* mvFlashInit - Initialize a flash descriptor structure.
*
* DESCRIPTION:
*       This function intialize flash info struct with specified flash
*       parameters. This structure is used to identify the target flash the
*       function refers to. This allow the use of the same API for multiple
*       flash devices.
*
*
* INPUT:
*       pFlash->baseAddr - Flash base address.
*       pFlash->busWidth - Flash bus width (8, 16, 32 bit).
*       pFlash->devWidth - Flash device width (8 or 16 bit).
*
* OUTPUT:
*       pFlash - Flash identifier structure.
*
* RETURN:
*       32bit describing flash size.
*       In case of any error, it returns 0.
*
*******************************************************************************/
MV_U32 mvFlashInit(MV_FLASH_INFO *pFlash)
{
	MV_U32 manu = 0, id = 0;

	if(NULL == pFlash)
		return 0;

	DB(mvOsOutput("Flash: mvFlashInit base 0x%x devW %d busW %d\n",
						pFlash->baseAddr, pFlash->devWidth, pFlash->busWidth));

	/* must init first sector base, before calling flashCmdSet */
	pFlash->sector[0].baseOffs = 0;
	/* reset flash 0xf0(AMD) 0xff (Intel) */
	flashCmdSet(pFlash, 0, 0, 0xf0);
	flashCmdSet(pFlash, 0, 0, 0xff);

	/* Write auto select command: read Manufacturer ID 	*/
	/* AMD seq is: 0x555 0xAA -> 0x2AA 0x55 -> 0x555 0x90 	*/
	/* INTEL seq is dc 0x90					*/
	flashCmdSet(pFlash, 0x555, 0, 0xAA);
	flashCmdSet(pFlash, 0x2AA, 0, 0x55);
	flashCmdSet(pFlash, 0x555, 0, 0x90);


	/* Write auto select command: read Manufacturer ID 	*/
	/* SST seq is: 0x5555 0xAA -> 0x2AAA 0x55 -> 0x5555 0x90 	*/
	flashCmdSet(pFlash, 0x5555, 0, 0xAA);
	flashCmdSet(pFlash, 0x2AAA, 0, 0x55);
	flashCmdSet(pFlash, 0x5555, 0, 0x90);

	DB(mvOsOutput("Flash: mvFlashInit base 0x%x devW %d busW %d\n",
						pFlash->baseAddr, pFlash->devWidth, pFlash->busWidth));


	/* get flash Manufactor and Id */
	manu = flashBusWidthRd(pFlash, mvFlashBaseAddrGet(pFlash));
	DB(mvOsOutput("Flash: mvFlashInit base 0x%x devW %d busW %d\n",
						pFlash->baseAddr, pFlash->devWidth, pFlash->busWidth));


	/* Some Micron flashes don't use A0 address for Identifier and
	Lock information, so in order to read Identifier and lock information
	properly we will do the following workarround:
	If our device width is 1 (x8) then if address 0 equal to address 1
	and address 2 equal to address 3 ,then we have this case (A0 is not used)
	and then we will issue the address without A0 to read the Identifier and
	lock information properly*/
	DB(mvOsOutput("Flash: mvFlashInit base 0x%x devW %d busW %d\n",
						pFlash->baseAddr, pFlash->devWidth, pFlash->busWidth));


	if ((pFlash->devWidth == 1) &&
		  ((flashBusWidthRd(pFlash, flashAddrExt(pFlash, 0, 0)) ==
			flashBusWidthRd(pFlash, flashAddrExt(pFlash, 1, 0)))&&
		   (flashBusWidthRd(pFlash, flashAddrExt(pFlash, 2, 0)) ==
			flashBusWidthRd(pFlash, flashAddrExt(pFlash, 3, 0)))))
		{
			id = flashBusWidthRd(pFlash, flashAddrExt(pFlash, 2, 0));

		} else id = flashBusWidthRd(pFlash, flashAddrExt(pFlash, 1, 0));


	/* check if this flash is Supported, and Init the pFlash flash feild */
	if( MV_OK != flashStructGet(pFlash, manu, id ) )
	{
		mvOsPrintf("%s: Flash ISN'T supported: manufactor-0x%x, id-0x%x\n",
                    __FUNCTION__, manu, id);
		return 0;
	}
	DB(mvOsOutput("Flash: mvFlashInit base 0x%x devW %d busW %d\n",
						pFlash->baseAddr, pFlash->devWidth, pFlash->busWidth));


	/* Init pFlash sectors */
	if(MV_OK != flashSecsInit(pFlash))
	{
		mvOsPrintf("Flash: ERROR mvFlashInit flashSecsInit failed \n");
		return 0;
	}

	DB(mvOsOutput("Flash: mvFlashInit base 0x%x devW %d busW %d\n",
						pFlash->baseAddr, pFlash->devWidth, pFlash->busWidth));


	/* print all flash information */
	DB(flashPrint(pFlash));

	/* reset the Flash */
	flashReset(pFlash);
	DB(mvOsOutput("Flash: mvFlashInit base 0x%x devW %d busW %d\n",
						pFlash->baseAddr, pFlash->devWidth, pFlash->busWidth));


	return mvFlashSizeGet(pFlash);
}


/* erase */
/*******************************************************************************
* mvFlashErase - Completly Erase a flash.
*
* DESCRIPTION:
*       This function completly erase the given flash, by erasing all the
*		flash sectors one by one (Currently there is no support for HW
*		flash erase).
*
* INPUT:
*       pFlash - Flash identifier structure.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BAD_PARAM if pFlash is NULL,
*   MV_OK if erased completed successfully,
*	MV_FAIL otherwise.
*
*******************************************************************************/
MV_STATUS mvFlashErase(MV_FLASH_INFO *pFlash)
{
	MV_U32 i;

	if(NULL == pFlash)
		return MV_BAD_PARAM;

	DB(mvOsPrintf("Flash: mvFlashErase \n"));
	/* erase all sectors in the flash one by one */
	for(i = 0; i < mvFlashNumOfSecsGet(pFlash); i++)
	{
		if( MV_OK != mvFlashSecErase(pFlash,i) )
			return MV_FAIL;
	}

	return MV_OK;
}

/*******************************************************************************
* flashIsSecErased - Check if a given Sector is erased.
*
* DESCRIPTION:
*	Go over the sector and check if its entire data is 0xFF.
* INPUT:
*       secNum	- sector Number.
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	MV_BAD_PARAM if one of the inputs values is illegal,
*	MV_TRUE if sector is already erased,
*	MV_FALSE otherwise.
*
*******************************************************************************/
MV_BOOL flashIsSecErased(MV_FLASH_INFO *pFlash, MV_U32 secNum)
{
	MV_U32 i;

	DB(mvOsPrintf("Flash: flashIsSecErased. \n"));
	if((NULL == pFlash) || (secNum >= mvFlashNumOfSecsGet(pFlash)) )
		return MV_BAD_PARAM;

	/* reset the flash */
	flashReset(pFlash);

	/* go over the sector */
	for(i = mvFlashSecOffsGet(pFlash,secNum);
		i < mvFlashSecOffsGet(pFlash,secNum) + mvFlashSecSizeGet(pFlash,secNum);
		i+= 4)
	{
		if(mvFlash32Rd(pFlash,i) != FLASH_WR_ERASED )
		{
			DB(mvOsPrintf("Flash: Not erased addr %x is %x \n",
											i ,mvFlash32Rd(pFlash,i)));
			return MV_FALSE;
		}
	}
	return MV_TRUE;
}


/*******************************************************************************
* mvFlashSecErase - Erase a flash sector.
*
* DESCRIPTION:
*       This function checks if the sector isn't protected and if the sector
*		isn't already erased.
*
* INPUT:
*       pFlash    - Flash identifier structure.
*       sectorNum - secrot number to erase.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BAD_PARAM if one of the inputs values is illegal,
*       MV_OK if erased completed successfully,
*	MV_FAIL otherwise (e.g. sector protected).
*
*******************************************************************************/
MV_STATUS mvFlashSecErase(MV_FLASH_INFO *pFlash, MV_U32 sectorNum)
{
	MV_U32 status;
	DB(mvOsPrintf("Flash: mvFlashSecErase \n"));

	/* check parametrs values */
	if((NULL == pFlash) || (sectorNum >= mvFlashNumOfSecsGet(pFlash)) ) {
		return MV_BAD_PARAM;
	}

	/* check if sector is locked */
	if(MV_TRUE == mvFlashSecLockGet(pFlash, sectorNum))
	{
		mvOsPrintf("Flash: ERROR mvFlashSecErase protected sector.\n");
		return MV_FAIL;
	}
	/* check if already erased */
	if(MV_TRUE == flashIsSecErased(pFlash,sectorNum))
	{
		DB(mvOsPrintf("Flash: FlashSecErase sector already erased \n"));
		return MV_OK;
	}

	/* erase sector using the Flash Ven Alg. */
	switch(mvFlashVenIdGet(pFlash))
	{
		case INTEL_MANUF: /* INTEL/MT */
			status = intelFlashSecErase(pFlash,sectorNum);
			break;
		case AMD_MANUF:
		case STM_MANUF:
		case SST_MANUF:
		case MX_MANUF:
			status = amdFlashSecErase(pFlash,sectorNum);
			break;
		default:
			mvOsPrintf("Flash: ERROR mvFlashErase. unsupported flash vendor\n");
			return MV_FAIL;
	}
	/* reset the flash */
	flashReset(pFlash);

	return status;
}

/* write */
/*******************************************************************************
* mvFlash32Wr - Write 32bit (word) to flash.
*
* DESCRIPTION:
*       This function writes 32bit data to a given flash offset.
*
* INPUT:
*       pFlash - Flash identifier structure (flash cockie).
*       offset - Offset from flash base address.
*       data   - 32bit data to be written to flash.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BAD_PARAM if one of the inputs values is illegal,
*       MV_OK if write completed successfully,
*	MV_FAIL otherwise (e.g. sector protected).
*
*******************************************************************************/
MV_STATUS mvFlash32Wr(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U32 data)
{
	MV_U32 i, status = MV_OK, temp,secNum;

	DB(mvOsPrintf("Flash: mvFlash32Wr offset %x data %x \n",offset,data));

	/* check that the offset is aligned to 32 bit */
	if((NULL == pFlash) || (offset % 4))
		return MV_BAD_PARAM;

	secNum = mvFlashInWhichSec(pFlash,offset);

	DB(mvOsPrintf("Flash: mvFlashProg \n"));
	/* check if offset is in flash range */
	if( secNum >= mvFlashNumOfSecsGet(pFlash))
	{
		DB(mvOsPrintf("Flash: mvFlashProg offset out of flash range \n"));
		return MV_BAD_PARAM;
	}

	/* check if sector is locked */
	if(MV_TRUE == mvFlashSecLockGet(pFlash, secNum) )
	{
		mvOsPrintf("Flash: ERROR mvFlashProg protected sector.\n");
		return MV_FAIL;
	}

	/* check if offset is erased enough */
	if((mvFlash32Rd(pFlash, offset) & data) != data )
	{
		mvOsPrintf("%s ERROR: offset 0x%x (sector %d) isn't erased !!!.\n",
                    __FUNCTION__, offset, secNum);
		return MV_FAIL;
	}


	/* bus width is 32 bit */
	if(mvFlashBusWidthGet(pFlash) == 4)
	{
		data = MV_32BIT_BE(data);
		status = mvFlashProg(pFlash, offset,data);
		if (status != MV_OK )
		{
			mvOsPrintf("%s ERROR: mvFlashProg() status %x \n",
                       __FUNCTION__, status);
		}
	}

	/* bus width is 16 bit */
	else if(mvFlashBusWidthGet(pFlash) == 2)
	{
		for(i = 0; i < 2; i++)
		{
			/* 0x44556677 -> [44][55][66][77] */
			temp = MV_16BIT_BE(((data >> (16*(1-i))) & FLASH_MASK_16BIT));
			if(MV_OK != mvFlashProg(pFlash, offset + (i*2), temp) )
			{
				status = MV_FAIL;
				break;
			}
		}
	}
	/* bus width is 8 bit */
	else if(mvFlashBusWidthGet(pFlash) == 1)
	{
		for(i = 0; i < 4; i++)
		{
			/* 0x44556677 -> [44][55][66][77] */
			temp = ((data >> (8*(3-i))) & FLASH_MASK_8BIT);
			if(MV_OK != mvFlashProg(pFlash, offset + i, temp ))
			{
				status = MV_FAIL;
			}
		}
	}
	/* bus width isn't 8/16/32 */
	else
	{
		DB(mvOsPrintf("Flash: mvFlashWordWr no support for for bus width %d \n",
												mvFlashBusWidthGet(pFlash) ));
		status = MV_FAIL;
	}
	/* reset the flash */
	flashReset(pFlash);

	if (status != MV_OK )
	{
		mvOsPrintf("mvFlash32Wr: ERROR #### status %x \n", status );
	}

	return status;
}

/*******************************************************************************
* mvFlash16Wr - Write 16bit (short) to flash.
*
* DESCRIPTION:
*       This function writes 16bit data to a given flash offset.
*
* INPUT:
*       pFlash - Flash identifier structure (flash cockie).
*       offset - Offset from flash base address.
*       sdata  - 16bit data to be written to flash.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BAD_PARAM if one of the inputs values is illegal,
*       MV_OK if write completed successfully,
*	MV_FAIL otherwise (e.g. sector protected).
*
*******************************************************************************/
MV_STATUS mvFlash16Wr(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U16 sdata)
{
	MV_U32 temp,shiftSdata;

	DB(mvOsPrintf("Flash: mvFlash16Wr\n"));

	/* check that the offset is aligned to 16 bit */
	if((NULL == pFlash) || (offset % 2))
		return MV_BAD_PARAM;

	/* sdata shift in 32 bit aligned. i.e. 									*/
	/* wr 0x9922 to 0xf4000002 -> 0xf4000000: qqqq9999 						*/
	shiftSdata = (1 - ((offset & 0x2)>>1) ) * 16;
	/* read 32 bit aligned */
	temp = mvFlash32Rd(pFlash, MV_ALIGN_DOWN(offset,4)); /* aligned to 32 bit */
	/* write 16 bit sdata into 32 bit */
	temp &= temp & ~(0xffff << shiftSdata);
	temp |= sdata << shiftSdata;

	/* write 32 bit */
	return mvFlash32Wr(pFlash,MV_ALIGN_DOWN(offset,4),temp);
}

/*******************************************************************************
* mvFlash8Wr - Write 8bit (char) to flash.
*
* DESCRIPTION:
*       This function writes 8bit data to a given flash offset.
*
* INPUT:
*       pFlash - Flash identifier structure (flash cockie).
*       offset - Offset from flash base address.
*       cdata  - 8bit data to be written to flash.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BAD_PARAM if one of the inputs values is illegal,
*       MV_OK if write completed successfully,
*	MV_FAIL otherwise (e.g. sector protected).
*
*******************************************************************************/
MV_STATUS mvFlash8Wr(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U8 cdata)
{
	MV_U32 temp, shiftCdata;

	DB(mvOsPrintf("Flash: mvFlash8Wr\n"));
	/* check that the offset is aligned to 16 bit */
	if(NULL == pFlash)
		return MV_BAD_PARAM;

	/* cdata shift in 32 bit aligned. i.e. 									*/
	/* wr 0x99 to 0xf4000003 -> 0xf4000000: qqqqqq99 						*/
	shiftCdata = (3 - (offset & 0x3)) * 8;
	/* read 32 bit aligned */
	temp = mvFlash32Rd(pFlash, MV_ALIGN_DOWN(offset,4)); /* aligned to 32 bit */
	/* write 16 bit sdata into 32 bit */
	temp &= temp & ~(0xff << shiftCdata);
	temp |= cdata << shiftCdata;
	/* write 32 bit */
	return mvFlash32Wr(pFlash,MV_ALIGN_DOWN(offset,4),temp);
}

/*******************************************************************************
* mvFlashBlockUnbufWr - Write a block to flash. Unbuffered
*
* DESCRIPTION:
*       This function writes a block of data to a given flash offset.
*
* INPUT:
*       pFlash    - Flash identifier structure (flash cockie).
*       offset    - Offset from flash base address.
*       blockSize - Size of block in bytes.
*       pBlock    - Pointer to data block to be written to flash.
*
* OUTPUT:
*       None.
*
* RETURN:
*       The number of bytes written to flash.
*
*******************************************************************************/
MV_U32 mvFlashBlockUnbufWr(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize,
                                 MV_U8 *pBlock)
{
	MV_U32 i, j, temp = 0;

	DB(mvOsPrintf("Flash: mvFlashBlockWr\n"));
	if(NULL == pFlash)
		return 0;

#ifndef CONFIG_MARVELL
	if(NULL == pBlock)
		return 0;
#endif

	for(i = 0; i < blockSize; )
	{
		if( ((offset + i) % 4) || ((blockSize - i) < 4) )/* unaligned to 32 bit*/
		{
			DB(mvOsPrintf("Flash: mvFlashBlockWr not aligned\n"));
			if(MV_OK != mvFlash8Wr(pFlash, offset + i, pBlock[i]) )
			{
				DB(mvOsPrintf("Flash: mvFlashBlockWr failed in writing char\n"));
				return i;
			}
			i++;
		}
		else /* aligned to 32 bit */
		{
			temp = 0;
			/* to make sure we don't write to un aligned address */
			for(j = 0; j < 4; j++)
			{
				/* [44][55][66][77] -> 0x44556677  */
				temp |=  (pBlock[i+j] & FLASH_MASK_8BIT ) << (8*(3-j)) ;
			}
			if( MV_OK != mvFlash32Wr(pFlash, offset + i, temp))
			{
				DB(mvOsPrintf("Flash: mvFlashBlockWr failed in writing word\n"));
				return i;
			}
			i += 4;
		}
	}

	return i;
}


/*******************************************************************************
* mvFlashBlockWr - Write a block to flash.
*
* DESCRIPTION:
*       This function writes a block of data to a given flash offset.
*
* INPUT:
*       pFlash    - Flash identifier structure (flash cockie).
*       offset    - Offset from flash base address.
*       blockSize - Size of block in bytes.
*       pBlock    - Pointer to data block to be written to flash.
*
* OUTPUT:
*       None.
*
* RETURN:
*       The number of bytes written to flash.
*
*******************************************************************************/
MV_U32 mvFlashBlockWr(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize,
                                 MV_U8 *pBlock)
{
	MV_U32  numOfBytesWritten = 0;
	MV_U32  secNum; /* lastOffset;*/
	MV_U32  i;
	MV_U32  hwBuffSize, sizeToWrite;
	MV_U32  unBufWritten= 0;
	MV_U8   *pTmpBlock = NULL;

	DB(mvOsPrintf("Flash: mvFlashBlockWr\n"));
	if(NULL == pFlash)
		return 0;

#ifndef CONFIG_MARVELL
	if(NULL == pBlock)
		return 0;
#endif

	/* check if any of the dest sectors is protected  */
	/*lastOffset = ((MV_U32)pBlock) + blockSize;*/
	secNum = mvFlashInWhichSec(pFlash,offset);
	do
	{
		if( MV_TRUE == mvFlashSecLockGet(pFlash, secNum) )
		{
			mvOsPrintf("Flash: ERROR mvFlashProg protected sector.\n");
			return 0;
		}

		/* next sec base offset */
		i = mvFlashSecOffsGet(pFlash, secNum) +
			mvFlashSecSizeGet(pFlash, secNum);
		secNum++;

	} while (i < blockSize);

	hwBuffSize = flashGetHwBuffSize(pFlash);

	/* if no HW buffer support, then call unbuffer routine*/
	if (hwBuffSize == 0)
	{
		return mvFlashBlockUnbufWr(pFlash, offset, blockSize,
                                 pBlock);

	}

	flashReset(pFlash);


	/* now write unbuffered the unaligned data*/
	while (((offset % hwBuffSize) || (blockSize < hwBuffSize))&&(blockSize))
	{
		DB(mvOsPrintf("if I: offset = 0x%x, blockSize= 0x%x\n",
					offset,blockSize));

		if ((blockSize < hwBuffSize)||
		   ((offset + blockSize) < (offset + (2*hwBuffSize + (offset / hwBuffSize)))))
		{
			sizeToWrite = blockSize;
		}
		else sizeToWrite = hwBuffSize - (offset % hwBuffSize);


		unBufWritten = mvFlashBlockUnbufWr(pFlash, offset, sizeToWrite,
                                 pBlock);

		flashReset(pFlash);

		blockSize -= unBufWritten;
		offset += unBufWritten;
		pBlock += unBufWritten;
		numOfBytesWritten += unBufWritten;

		if (unBufWritten != sizeToWrite) return numOfBytesWritten;


	}

	if (blockSize)
	{
		/* now write buffered the aligned data*/
		sizeToWrite = blockSize - (blockSize % hwBuffSize);

		/* Check source addr, in case source is in the FLASH 		*/
		/* First copy the data to a temporary container in the SDRAM 	*/
		/* and only then copy the data from the SDRAM to the FLASH 	*/
		if (((MV_U32)pBlock >= mvFlashBaseAddrGet(pFlash)) &&
			((MV_U32)pBlock < (mvFlashBaseAddrGet(pFlash) + mvFlashSizeGet(pFlash))))
		{
			/* Malloc memory */
			if (NULL == (pTmpBlock=mvOsMalloc(sizeToWrite)))
			{
				DB(mvOsPrintf("mvFlashBlockWr: Malloc temporary container failed\n"));
				flashReset(pFlash);
				return numOfBytesWritten;
			}
			/* Copy data to SDRAM */
			memcpy(pTmpBlock, pBlock, sizeToWrite);
			/* Change source pointer to SDRAM */
		}
		else
			/* Source address is not in flash */
			pTmpBlock = pBlock;

		/* Write data to flash */
		if ( MV_OK != flashHwBufferProg(pFlash, offset,
										sizeToWrite, pTmpBlock) )
		{
			DB(mvOsPrintf("mvFlashBlockWr: flashHwBufferProg failed\n"));
			flashReset(pFlash);
			return numOfBytesWritten;
		}
		/* Free memory only if used SDRAM container */
		if (pTmpBlock != pBlock)
			mvOsFree(pTmpBlock);

		flashReset(pFlash);

		blockSize -= sizeToWrite;
		offset += sizeToWrite;
		pBlock += sizeToWrite;
		numOfBytesWritten += sizeToWrite;

	}


	/* now write unbuffered the rest*/
	if (blockSize)
	{
		unBufWritten = mvFlashBlockUnbufWr(pFlash, offset, blockSize,
                                 pBlock);

		flashReset(pFlash);

		blockSize -= unBufWritten;
		offset += unBufWritten;
		pBlock += unBufWritten;
		numOfBytesWritten += unBufWritten;

	}


	return numOfBytesWritten;
}


/* read */
/*******************************************************************************
* mvFlash32Rd - Read a 32bit (word) from flash.
*
* DESCRIPTION:
*       This function reads 32bit (word) data from a given flash offset.
*
* INPUT:
*       pFlash   - Flash identifier structure (flash cockie).
*       offset   - Offset from flash base address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit data read from flash.
*
*******************************************************************************/
MV_U32 mvFlash32Rd(MV_FLASH_INFO *pFlash, MV_U32 offset)
{
	MV_U32 val;

	DB(mvOsPrintf("Flash: mvFlashWordRd %x\n",offset));
	if(NULL == pFlash)
		return 0;

	val = MV_FL_32_DATA_READ(offset + mvFlashBaseAddrGet(pFlash));

	return MV_32BIT_BE(val);
}

/*******************************************************************************
* mvFlash16Rd - Read a 16bit (short) from flash.
*
* DESCRIPTION:
*       This function reads 16bit (short) data from a given flash offset.
*
* INPUT:
*       pFlash   - Flash identifier structure (flash cockie).
*       offset   - Offset from flash base address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       16bit data read from flash.
*
*******************************************************************************/
MV_U16 mvFlash16Rd (MV_FLASH_INFO *pFlash,MV_U32 offset)
{
	MV_U32 val;

	if(NULL == pFlash)
		return 0;

	val = MV_FL_16_DATA_READ(offset + mvFlashBaseAddrGet(pFlash));

	return MV_16BIT_BE(val);
}

/*******************************************************************************
* mvFlash8Rd - Read a 8bit (char) from flash.
*
* DESCRIPTION:
*       This function reads 8bit (char) data from a given flash offset.
*
* INPUT:
*       pFlash   - Flash identifier structure (flash cockie).
*       offset   - Offset from flash base address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       8bit data read from flash.
*	0 if pflash is NULL.
*
*******************************************************************************/
MV_U8 mvFlash8Rd  (MV_FLASH_INFO *pFlash, MV_U32 offset)
{
	if(NULL == pFlash)
		return 0;

	return MV_FL_8_DATA_READ(offset + mvFlashBaseAddrGet(pFlash));
}

/*******************************************************************************
* mvFlashBlockRd - Read a block of data from flash.
*
* DESCRIPTION:
*       This function reads a block of data from given flash offset.
*
* INPUT:
*       pFlash    - Flash identifier structure (flash cockie).
*       offset    - Offset from flash base address.
*       blockSize - Size of block in bytes.
*
* OUTPUT:
*       pBlock   - Pointer to data block to be read from flash.
*
* RETURN:
*       The number of bytes read from flash.
*
*******************************************************************************/
MV_U32 mvFlashBlockRd (MV_FLASH_INFO *pFlash, MV_U32 offset,MV_U32 blockSize,
                       MV_U8 *pBlock)
{
	MV_U32 i, j, temp;

	if((NULL == pFlash) || (NULL == pBlock))
		return 0;

	for(i = 0; i < blockSize; )
	{
		if( ((offset + i) % 4) || ((blockSize - i) < 4) )/* unaligned to 32 bit*/
		{
			DB(mvOsPrintf("Flash: mvFlashBlockRd not aligned\n"));
			pBlock[i] = mvFlash8Rd(pFlash, offset + i);
			i++;
		}
		else /* aligned to 32 bit */
		{
			temp = mvFlash32Rd(pFlash, offset + i);
			/* to make sure we don't write to un aligned address */
			for(j = 0; j < 4; j++)
			{
				/* 0x44556677 -> [44][55][66][77] */
				pBlock[i+j] = (MV_U8)((temp >> (8*(3-j))) & FLASH_MASK_8BIT);
			}
			i += 4;
		}
	}

	return i;
}

/*******************************************************************************
* mvFlashSecLockSet - Lock/Unlock a Sector in the flash for Writing.
*
* DESCRIPTION:
*       Lock/Unlock a Sector in the flash for Writing.
*
* INPUT:
*       pFlash	- Flash identifier structure.
*       secNum	- Sector Number.
*       enable	- MV_TRUE for Lock MV_FALSE for un-lock.
*
* OUTPUT:
*      None
*
* RETURN:
*	MV_BAD_PARAM if pFlash is NULL,
*       MV_OK if operation completed successfully,
*	MV_FAIL otherwise
*
*******************************************************************************/
MV_STATUS mvFlashSecLockSet(MV_FLASH_INFO *pFlash, MV_U32 secNum, MV_BOOL enable)
{
	MV_U32 status = MV_FAIL;

	DB(mvOsPrintf("Flash: mvFlashSecLockSet\n"));
	if( (NULL == pFlash) || (secNum > mvFlashNumOfSecsGet(pFlash)) )
		return MV_BAD_PARAM;

	/* check if sector is locked */
	if(enable == mvFlashSecLockGet(pFlash, secNum) )
	{
		DB(mvOsPrintf("already un/locked\n"));
		return MV_OK;
	}

	/* SW Lock */
	if( mvFlashIsHwLock(pFlash) == MV_FALSE)
	{
		pFlash->sector[secNum].protect = enable;
		status = MV_OK;
	}
	else /* HW Lock */
	{
		switch(mvFlashVenIdGet(pFlash))
		{
			case INTEL_MANUF: /* INTEL / MT */
				status = intelFlashSecLock(pFlash,secNum,enable);
				break;
			default:
				mvOsPrintf("%s ERROR: No support for flash vendor id=0x%x\n",
                            __FUNCTION__, mvFlashVenIdGet(pFlash));
				return MV_FAIL;
		}
		/* if completed successfully, updated SW structure */
		if( MV_OK == status )
		{
			pFlash->sector[secNum].protect = enable;
		}

		/* reset the flash */
		flashReset(pFlash);
	}

	return status;
}

/********************** statics APIs *******************************************/
/*******************************************************************************
* flashReset - Reset the Flash.
* DESCRIPTION:
*	Reset the flash (Inset it into read mode).
*
* INPUT:
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	MV_FAIL - if flash Vendor isn't supported MV_OK otherwise.
*
*******************************************************************************/
static MV_STATUS flashReset(MV_FLASH_INFO *pFlash)
{
	switch(mvFlashVenIdGet(pFlash))
	{
		case INTEL_MANUF: /* INTEL/MT */
			intelFlashReset(pFlash);
			break;
		case AMD_MANUF:
		case STM_MANUF:
		case SST_MANUF:
			amdFlashReset(pFlash);
			break;
		default:
			mvOsPrintf("Flash: ERROR mvFlashErase unsupported flash vendor\n");
			return MV_FAIL;
	}
	return MV_OK;
}

/*******************************************************************************
* flashStructGet - return flash structure information.
*
* DESCRIPTION:
*       This function goes over the supported flash list and look for a flash
*	with manufactor code = manu and id code = id if found it return the flash
*	structure.
*
* INPUT:
*       manu    - Flash Manufactor.
*       id    	- Flash ID.
*
* OUTPUT:
*       pFlash	- Flash structure.
*
* RETURN:
*	MV_BAD_PARAM if pFlash is NULL,
*       MV_OK if manufactor and id were found,
*	MV_FAIL otherwise
*
*******************************************************************************/
static MV_STATUS flashStructGet(MV_FLASH_INFO *pFlash, MV_U32 manu, MV_U32 id)
{
	MV_U32 i= 0;

	if(NULL == pFlash)
		return MV_BAD_PARAM;

	DB(mvOsPrintf("Flash: flashStructGet manu 0x%x id 0x%x \n",manu,id));
	/* while its not the last supported flash */
	while((supFlashAry[i].flashVen != LAST_FLASH) ||
		  (supFlashAry[i].flashId != LAST_FLASH))
	{
		/* if supported flash manufactor and Id equal to manu and id break */
		if( (flashDataExt(pFlash, supFlashAry[i].flashVen) == manu) &&
		    (flashDataExt(pFlash, supFlashAry[i].flashId) == id)  )
		{
			DB(mvOsPrintf("Flash: flashStructGet flash is supported.\n"));
			pFlash->flashSpec = supFlashAry[i];
			return MV_OK;
		}
		i++;
	}

	/* manu and id are not supported! */
	DB(mvOsPrintf("Flash: flashStructGet flash is not supported.\n"));
	return MV_FAIL;
}


/*******************************************************************************
*  flashSecsInit - Init the flash sector array in pFlash.
*
* DESCRIPTION:
*	Init the sector array based on the sector type.
*
* INPUT:
*
* OUTPUT:
*       pFlash	- Flash sectors information.
*
* RETURN:
*	MV_BAD_PARAM if pFlash or frag sector struct(if needed) are NULL,
*	MV_OK otherwise
*
*******************************************************************************/
static MV_STATUS flashSecsInit(MV_FLASH_INFO *pFlash)
{
	MV_U32 	i, temp, base = 0;
	MV_U32	restSecSize, initSize = 0;
	MV_U32	*pSecFrag, numSecFrag, firstSec, lastSec , flashDevNum;

	if(NULL == pFlash)
		return MV_BAD_PARAM;

	/* first Init the bottom or top sectors */
	if( mvFlashSecTypeGet(pFlash) != REGULAR ) /* BOTTOM or TOP */
	{
		/* init sectors fragments parameters */
		numSecFrag = pFlash->flashSpec.secFragNum;
		pSecFrag   = pFlash->flashSpec.pSecSizeFragList;
		if(NULL == pSecFrag)
		{
			mvOsPrintf("Flash: flashSecsInit missing frag sector list.\n");
			return MV_BAD_PARAM;
		}

		/* In case we got more then one flash in parallel then each 		*/
		/* fragment size will be "duplicated" 								*/
		flashDevNum = mvFlashNumOfDevGet(pFlash);

		/* caculate the size of each sector in the rest of the flash sector */
		temp = 0;
		for(i = 0; i < numSecFrag; i++)
		{
			temp += pSecFrag[i] * flashDevNum;
		}
		restSecSize = (mvFlashSizeGet(pFlash) - temp) /
			          (mvFlashNumOfSecsGet(pFlash) - numSecFrag);


		if(mvFlashSecTypeGet(pFlash) == TOP) /* TOP */
		{
			/* if TOP sec type the the last sector is fragmented */
			DB(mvOsPrintf("FLASH: initFlashSecs TOP Sector Type \n"));
			base = mvFlashSizeGet(pFlash) - restSecSize;
			temp = mvFlashNumOfSecsGet(pFlash) - numSecFrag;

			for(i = 0; i < numSecFrag; i++)
			{
				pFlash->sector[temp + i].baseOffs = base;
				pFlash->sector[temp + i].size =
					               pSecFrag[(numSecFrag - 1)- i] * flashDevNum;
				/* Init Protect feild */
				if(pFlash->flashSpec.HwProtect == MV_FALSE) /* SW Protect */
				{
					pFlash->sector[i].protect = MV_FALSE;
				}
				else/* HW protect */
				{
					pFlash->sector[i].protect=flashSecLockGet(pFlash, temp + i);
				}
				/*increment base and size of initialized sectors */
				base += pSecFrag[(numSecFrag - 1) - i] * flashDevNum;
				initSize += pSecFrag[(numSecFrag - 1) - i] * flashDevNum;
			}
			/* prepare for rest of sector init */
			firstSec = 0;
			lastSec = mvFlashNumOfSecsGet(pFlash) - numSecFrag;
			base = 0;

		}
		else if(mvFlashSecTypeGet(pFlash) == BOTTOM)/* BOTTOM */
		{
			/* if BOTTOM sec type the the first sector is fragmented */
			DB(mvOsPrintf("FLASH: initFlashSecs BOTTOM Sector Type \n"));

			for(i = 0; i < numSecFrag; i++)
			{
				pFlash->sector[i].baseOffs = base;
				pFlash->sector[i].size = pSecFrag[i] * flashDevNum;
				/* Init Protect feild */
				if(pFlash->flashSpec.HwProtect == MV_FALSE) /* SW Protect */
				{
					pFlash->sector[i].protect = MV_FALSE;
				}
				else/* HW protect */
				{
					pFlash->sector[i].protect = flashSecLockGet(pFlash, i);
				}
				/*increment base and size of initialized sectors */
				base += pSecFrag[i] * flashDevNum;
				initSize += pSecFrag[i] * flashDevNum;
			}
			/* prepare for rest of sector init */
			firstSec = numSecFrag;
			lastSec = mvFlashNumOfSecsGet(pFlash);
		}
		else	/* unknown Type */
		{
			mvOsPrintf("FLASH: initFlashSecs Sector Type %d is unsupported\n.",
													pFlash->flashSpec.secType);
			return MV_BAD_PARAM;
		}

	}
	else /* REGULAR */
	{
		DB(mvOsPrintf("FLASH: initFlashSecs REGULAR Sector Type \n"));
		restSecSize = mvFlashSizeGet(pFlash) / mvFlashNumOfSecsGet(pFlash);
		firstSec = 0;
		lastSec = mvFlashNumOfSecsGet(pFlash);
	}

	/* init the rest of the sectors */
	DB(mvOsPrintf("Flash: flashSecsInit main sector loop %d - %d \n",
															firstSec,lastSec));
	for(i = firstSec; i < lastSec; i++)
	{
		pFlash->sector[i].baseOffs = base;
		pFlash->sector[i].size = restSecSize;
		/* Init Protect feild */
		if(pFlash->flashSpec.HwProtect == MV_FALSE) /* SW Protect */
		{
			pFlash->sector[i].protect = MV_FALSE;
		}
		else/* HW protect */
		{
			pFlash->sector[i].protect = flashSecLockGet(pFlash, i);
		}
		/*increment base */
		base += restSecSize;
	}

	return MV_OK;
}

/*******************************************************************************
* flashSecLockGet - Return a sector Lock Bit status.
*
* DESCRIPTION:
*	Return a sector Lock Bit status.
*
*
* INPUT:
*       pFlash	- Flash structure.
*       secNum 	- Sector Number.
*
* OUTPUT:
*	None.
*
* RETURN:
*       MV_TRUE if lock is set
*	MV_FALSE if lock isn't set
*
*******************************************************************************/
static MV_BOOL flashSecLockGet(MV_FLASH_INFO *pFlash, MV_U32 secNum)
{
	MV_U32 status;

	/* check if sector is locked */
	if( ( NULL == pFlash) || (secNum >= mvFlashNumOfSecsGet(pFlash)) )
		return MV_BAD_PARAM;

	switch(mvFlashVenIdGet(pFlash))
	{
		case INTEL_MANUF: /* INTEL / MT */
			status = intelFlashSecLockGet(pFlash,secNum);
			break;
		default:
			mvOsPrintf("Flash: ERROR flashSecLockGet unsupported vendor\n");
			return MV_FAIL;
	}
	/* reset the flash */
	flashReset(pFlash);

	return status;

}

/*******************************************************************************
* mvFlashProg - Prog busWidth Bits into the address offest in the flash.
*
* DESCRIPTION:
*       This function writes busWidth data to a given flash offset.
*
* INPUT:
*       pFlash - Flash identifier structure (flash cockie).
*       offset - Offset from flash base address.
*       data   - 32bit data to be written to flash.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BAD_PARAM if one of the inputs values is illegal,
*       MV_OK if program completed successfully,
*	MV_FAIL otherwise (e.g. sector protected).
*
*******************************************************************************/
static MV_STATUS mvFlashProg(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U32 data)
{
	MV_U32 status;


	switch(mvFlashVenIdGet(pFlash))
	{
		case INTEL_MANUF: /* INTEL / MT */
			status = intelFlashProg(pFlash,offset,data);
			break;
		case AMD_MANUF:
		case STM_MANUF:
		case SST_MANUF:
			status = amdFlashProg(pFlash,offset,data);
			break;
		default:
			mvOsPrintf("Flash: ERROR mvFlashProg unsupported flash vendor\n");
			return MV_FAIL;
	}

	return status;

}


/*******************************************************************************
* flashHwBufferProg - Prog flash via hw flash hw buffer
*
* DESCRIPTION:
*       This function writes to a given flash offset using flash hw buffer.
*
* INPUT:
*       pFlash - Flash identifier structure (flash cockie).
*       offset - Offset from flash base address.
*       data   - 32bit data to be written to flash.
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_BAD_PARAM if one of the inputs values is illegal,
*   MV_OK if program completed successfully,
*	MV_FAIL otherwise (e.g. sector protected).
*
*******************************************************************************/
static MV_STATUS flashHwBufferProg(MV_FLASH_INFO *pFlash, MV_U32 offset, MV_U32 byteCount,
																			MV_U8 *pData)
{
	MV_U32 status;


	switch(mvFlashVenIdGet(pFlash))
	{
		case INTEL_MANUF: /* INTEL / MT */
			status = intelFlashHwBufferProg(pFlash, offset, byteCount, pData);
			break;
		default:
			mvOsPrintf("Flash: ERROR flashHwBufferProg unsupported flash vendor\n");
			return MV_FAIL;
	}

	return status;
}


/*******************************************************************************
* flashGetHwBuffSize - get supported flash write buffer size.
* DESCRIPTION:
*	Returns supported flash write buffer size.
*
* INPUT:
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	MV_U32 - supported buffer size
*
*******************************************************************************/
static MV_U32 flashGetHwBuffSize(MV_FLASH_INFO *pFlash)
{

	switch(mvFlashVenIdGet(pFlash))
	{
		case INTEL_MANUF: /* INTEL / MT */
			return 	intelFlashGetHwBuffSize(pFlash);
			break;
		default:
			return 0;
	}
}
