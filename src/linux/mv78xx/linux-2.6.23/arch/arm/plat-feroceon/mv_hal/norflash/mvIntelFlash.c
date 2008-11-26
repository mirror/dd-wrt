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

#undef MV_DEBUG

#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

static MV_STATUS intelFlashStsGet(MV_FLASH_INFO *pFlash, MV_U32 sec,
								  MV_BOOL enableReadCommand, MV_U32* flashStatus);
static MV_VOID intelFlashStatusClr(MV_FLASH_INFO *pFlash);

/*******************************************************************************
* reset the flash
*******************************************************************************/
MV_VOID intelFlashReset(MV_FLASH_INFO *pFlash)
{
	flashCmdSet(pFlash, 0, 0, INTEL_CHIP_CMD_RST);
	return;
}

/*******************************************************************************
* intelFlashSecErase - Erase a sector.
*
* DESCRIPTION:
*	Erase a Flash sector.
*
* INPUT:
*       secNum	- sector Number.
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*       MV_OK if program completed successfully,
*	MV_TIMEOUT if timeout reached,
*	MV_FAIL otherwise.
*
*******************************************************************************/
MV_STATUS intelFlashSecErase(MV_FLASH_INFO *pFlash, MV_U32 secNum)
{
	MV_U32 i, status, flashStatus;

	DB(mvOsPrintf("Flash: intelFlashSecErase\n"));

	/* clear status */
	intelFlashStatusClr(pFlash);

	/* erase sequence */
	flashCmdSet(pFlash, 0, secNum, INTEL_CHIP_CMD_ERASE1);
	flashCmdSet(pFlash, 0, secNum, INTEL_CHIP_CMD_ERASE2);

	/* wait for erase to complete */
	for(i = 0; i < INTEL_EARASE_MILI_TIMEOUT; i++)
	{
		mvOsDelay(1);
		status = intelFlashStsGet(pFlash, 0, MV_TRUE, &flashStatus);
		if( MV_NOT_READY != status )
			return status;
	}
	mvOsPrintf("Flash: ERROR intelFlashSecErase timeout \n");	
	
	return MV_TIMEOUT;
}

/*******************************************************************************
* intelFlashProg - Prog busWidth Bits into the address offest in the flash.
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
*       MV_OK if program completed successfully,
*	MV_TIMEOUT if timeout reached,
*	MV_FAIL otherwise.
*
*******************************************************************************/
MV_STATUS intelFlashProg(MV_FLASH_INFO *pFlash,MV_U32 offset, MV_U32 data)
{
	MV_U32 i, status, flashStatus;

	DB(mvOsPrintf("Flash: mvIntelFlashWr\n"));

	/* clear status */
	intelFlashStatusClr(pFlash);

	/* write sequence */
	flashCmdSet(pFlash, 0, 0, INTEL_CHIP_CMD_PROG);
	flashBusWidthDataWr(pFlash, offset + mvFlashBaseAddrGet(pFlash), data);

	/* wait for write to complete */
	for(i = 0; i < INTEL_PROG_TIMEOUT; i++)
	{
		status = intelFlashStsGet(pFlash, 0, MV_TRUE, &flashStatus);
		if( MV_NOT_READY != status )
			return status;
	}
	mvOsPrintf("Flash: ERROR intelFlashSecErase timeout \n");	
	
	return MV_TIMEOUT;
}


MV_U32 intelFlashGetHwBuffSize(MV_FLASH_INFO *pFlash)
{
	MV_U32 buffSize;

	buffSize =	mvFlashNumOfDevGet(pFlash) * pFlash->flashSpec.HwBuffLen ;
	return buffSize;
}

/*******************************************************************************
* intelFlashBufferProg - Prog hw buff into the address offest in the flash.
*
* DESCRIPTION:
*       This function writes busWidth data to a given flash offset.
*
* INPUT:
*       pFlash - Flash identifier structure (flash cockie).
*       offset - Offset from flash base address.
*       pData  - buffer (32 byte) data to be written to flash.
*
* OUTPUT:
*       None.
*
* RETURN:
*   MV_OK if program completed successfully,
*   MV_BAD_PARAM illegal param
*	MV_TIMEOUT if timeout reached,
*	MV_FAIL otherwise.
*
*******************************************************************************/
MV_STATUS intelFlashHwBufferProg(MV_FLASH_INFO *pFlash,MV_U32 offset, 
								 MV_U32 byteCount, MV_U8* pData)
{
	MV_U32 i, status, flashStatus;
	MV_U32 buffSize, sec, wordCount, busWidth;
	MV_U32 offInBuff, absOffInFlash;
	MV_U32 data, j;

	DB(mvOsPrintf("Flash: intelFlashBufferProg\n"));

	if ((buffSize = (intelFlashGetHwBuffSize(pFlash) )) == 0)
	{
		return MV_BAD_PARAM;
	}
		
	if (0 != (byteCount % buffSize))
	{
		return MV_BAD_PARAM;
	}


	
	/* has to be aligned */
	if ((offset % buffSize) != 0)
	{
		return MV_BAD_PARAM;
	}

#ifndef CONFIG_MARVELL
	if (pData == NULL)
	{
		return MV_BAD_PARAM;
	}
#endif

	busWidth = mvFlashBusWidthGet(pFlash);
	wordCount = (pFlash->flashSpec.HwBuffLen / mvFlashDevWidthGet(pFlash));
	absOffInFlash = mvFlashBaseAddrGet(pFlash) + offset;
	offInBuff = 0;

	/* clear status */
	intelFlashStatusClr(pFlash);

	do
	{

		/* Start the 'write to buffer' sequence */
		sec = mvFlashInWhichSec(pFlash,offset);
	
		i=0;
		do
		{
			flashCmdSet(pFlash, 0, sec, INTEL_CHIP_CMD_WR_BUF);
			
			status = intelFlashStsGet(pFlash, sec, MV_FALSE, &flashStatus);
	
			if((i++ > INTEL_PROG_TIMEOUT)&&(status != MV_OK))
			{
				DB(printf("intelFlashHwBufferProg1: Timeout:offset = 0x%x flashStatus =0x%x\n",
						offset,flashStatus));
				return status;
			}
		
		}while (status != MV_OK);

        
		/* write num of words to write minus 1 */
		flashCmdSet(pFlash, 0, sec, wordCount - 1);
		
		for (i = 0; i < wordCount * mvFlashNumOfDevGet(pFlash); i += mvFlashNumOfDevGet(pFlash))
		{  
			data = 0;
			switch (busWidth)
			{
			case 4:
				for(j = 0; j < 4; j++)
				{
					/* [44][55][66][77] -> 0x44556677  */
					data |=  (pData[offInBuff+j] & FLASH_MASK_8BIT ) << (8*(3-j)) ;
				}
				data = MV_32BIT_BE(data);
				break;
			case 2:
				for(j = 0; j < 2; j++)
				{
					/* [44][55] -> 0x4455  */
					data |=  (pData[offInBuff+j] & FLASH_MASK_8BIT ) << (8*(1-j)) ;
				}
				data = MV_16BIT_BE(data);
				break;
			case 1:
				data = *(MV_U8*)(((MV_U32)pData) + offInBuff);
				break;
			}
			flashBusWidthDataWr(pFlash, absOffInFlash, data);
			
			offInBuff  += busWidth;
			absOffInFlash += busWidth;
		}

		flashCmdSet(pFlash, 0, sec, INTEL_CHIP_CMD_CONFIRM_BUF);

		/* check status  */
		for(i = 0; i < INTEL_PROG_TIMEOUT; i++)
		{
			status = intelFlashStsGet(pFlash, sec, MV_FALSE, &flashStatus);
			if ( status == MV_OK )
				break;
		}
		
		if ( status != MV_OK ) break;

		byteCount -= buffSize;
	}while(byteCount);


	if ( status != MV_OK )
	{

		DB(printf("intelFlashHwBufferProg2: Timeout:offset = 0x%x flashStatus =0x%x\n",
				offset,flashStatus));

		return status;
	}

		
	return MV_OK;
}


/*******************************************************************************
* intelSecLockGet - Return a sector Lock Bit status.
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
MV_BOOL intelFlashSecLockGet(MV_FLASH_INFO *pFlash,MV_U32 secNum)
{
	MV_U32  status;

	DB(mvOsPrintf("Flash: intelSecLockGet sector %d\n",secNum));

	/* clear status */
	intelFlashStatusClr(pFlash);

	/* Some Micron flashes don't use A0 address for Identifier and 
	Lock information, so in order to read Identifier and lock information
	properly we will do the following workarround:
	If our device width is 1 (x8) then if address 0 equal to address 1
	and address 2 equal to address 3 ,then we have this case (A0 is not used)
	and then we will issue the address without A0 to read the Identifier and
	lock information properly*/

	/* read Query sequence */
	flashCmdSet(pFlash, 0, 0, INTEL_CHIP_CMD_RD_QUERY);

	if ((pFlash->devWidth == 1) && 
		  ((flashBusWidthRd(pFlash, flashAddrExt(pFlash, 0, 0)) == 
			flashBusWidthRd(pFlash, flashAddrExt(pFlash, 1, 0)))&&
		   (flashBusWidthRd(pFlash, flashAddrExt(pFlash, 2, 0)) == 
			flashBusWidthRd(pFlash, flashAddrExt(pFlash, 3, 0)))))
		{
			status = flashBusWidthRd(pFlash, flashAddrExt(pFlash, 4, secNum));

		} else status = flashBusWidthRd(pFlash, flashAddrExt(pFlash, 2, secNum));


	if((status & flashDataExt(pFlash,INTEL_CHIP_RD_ID_LOCK)) != 0)
	{
		DB(mvOsPrintf("Flash: intelSecLockGet sector %d is locked \n",secNum));
		return MV_TRUE;
	}

	return MV_FALSE;
}
/*******************************************************************************
* intelSecLock - Lock/Unlock a Sector in the flash for Writing.
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
*       MV_OK if program completed successfully,
*	MV_TIMEOUT if timeout reached,
*	MV_FAIL otherwise.
*
*******************************************************************************/
MV_STATUS intelFlashSecLock(MV_FLASH_INFO *pFlash, MV_U32 secNum, MV_BOOL enable)
{
	MV_U32  status, intelLockEna, i, flashStatus;

	status = MV_ERROR;

	DB(mvOsPrintf("Flash: intelSecLock"));

	/* lock */
	if(enable == MV_TRUE)
	{
		DB(mvOsPrintf("Flash: intelSecLock Lock sector %d \n", secNum));
		intelLockEna = INTEL_CHIP_CMD_SET_LOCK_BLK;
	}
	else{ /* unlock */
		DB(mvOsPrintf("Flash: intelSecLock Unlock sector %d \n", secNum));
		intelLockEna = INTEL_CHIP_CMD_CLR_LOCK_BLK;
	}

	/* clear status */
	intelFlashStatusClr(pFlash);

	/* Un/lock sequence */
	flashCmdSet(pFlash, 0, 0, INTEL_CHIP_CMD_LOCK);
	flashCmdSet(pFlash, 0, secNum, intelLockEna);

	/* wait for write to complete */
	for(i = 0; i < INTEL_LOCK_MILI_TIMEOUT; i++)
	{
		mvOsDelay(1);
		status = intelFlashStsGet(pFlash, 0, MV_TRUE, &flashStatus);
		if(status != MV_NOT_READY)
			break;
	}
	/* if timeout */
	if(i >= INTEL_LOCK_MILI_TIMEOUT)
	{
		mvOsPrintf("Flash: ERROR intelSecLock timeout \n");	
		return MV_TIMEOUT;
	}
	/* if completed successfully */
	if( MV_OK == status )
	{
		/* Unprotect one sector, which means unprotect all flash
	 	* and reprotect the other protected sectors.
	 	*/
		if(enable == MV_FALSE)
		{
			for(i = 0; i < mvFlashNumOfSecsGet(pFlash); i++)
				if((i != secNum) && (MV_TRUE == mvFlashSecLockGet(pFlash,i)))
					if(MV_OK != intelFlashSecLock(pFlash,i,MV_TRUE))
						return MV_FAIL;
		}
		return MV_OK;
	}
	/* otherwise FAIL */	
	return MV_FAIL;
}

/*******************************************************************************
* Check the Flash Status and return:
*	MV_OK if status is ready and there isn't any error.
*	MV_FAIL if status is ready and there is an error.
*	MV_NOT_READY if status isn't ready.
*******************************************************************************/
static MV_STATUS intelFlashStsGet(MV_FLASH_INFO *pFlash ,MV_U32 sec,
								  MV_BOOL enableReadCommand, MV_U32* flashStatus)
{
	MV_U32	status;

	if ( enableReadCommand )
		flashCmdSet(pFlash, 0, 0, INTEL_CHIP_CMD_RD_STAT);
	status = flashBusWidthRd(pFlash, pFlash->baseAddr + mvFlashSecOffsGet(pFlash,sec));

	*flashStatus = status;

	if((status & flashDataExt(pFlash,INTEL_CHIP_STAT_RDY)) == 
                                    flashDataExt(pFlash,INTEL_CHIP_STAT_RDY))
	{
		DB(mvOsPrintf("Flash: intelFlashStatusChk value is ready \n"));
		if( (status & flashDataExt(pFlash,INTEL_CHIP_STAT_ERR)) != 0)
		{
			mvOsPrintf("Flash: intelFlashStatusChk value has ERROR %x \n", 
																		status);
			return MV_FAIL;
		}
		return MV_OK;
	}

	DB(mvOsPrintf("Flash: intelFlashStatusChk staus is %x and should %x \n",
                            status,flashDataExt(pFlash,INTEL_CHIP_STAT_RDY))); 
	return MV_NOT_READY;
}

/*******************************************************************************
* Clear the Flash Status.
*******************************************************************************/
static MV_VOID intelFlashStatusClr(MV_FLASH_INFO *pFlash)
{
	flashCmdSet(pFlash, 0, 0, INTEL_CHIP_CMD_CLR_STAT);
	return;
}

