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

static MV_BOOL amdFlashStsIsRdy(MV_FLASH_INFO *pFlash, MV_U32 offset, 
                                                              MV_U32 excepted);
static MV_VOID amdFlashUnlock(MV_FLASH_INFO *pFlash);

/*******************************************************************************
* reset the flash
*******************************************************************************/
MV_VOID amdFlashReset(MV_FLASH_INFO *pFlash)
{
    flashCmdSet(pFlash, 0, 0, AMD_CHIP_CMD_RST);
    return;
}

/*******************************************************************************
* amdFlashSecErase - Erase a sector.
*
* DESCRIPTION:
*       Erase a Flash sector.
*
* INPUT:
*       secNum  - sector Number.
*       pFlash  - flash information.
*
* OUTPUT:
*       None
*
* RETURN:
*       MV_OK if program completed successfully,
*       MV_TIMEOUT if timeout reached,
*       MV_FAIL otherwise.
*
*******************************************************************************/
MV_STATUS amdFlashSecErase(MV_FLASH_INFO *pFlash, MV_U32 secNum)
{
    MV_U32 i;
    
    DB(mvOsPrintf("Flash: amdFlashSecErase\n"));
    
    /* erase sequence */
    amdFlashUnlock(pFlash);

	switch (pFlash->flashSpec.flashVen)
	{
		case AMD_MANUF:
		case STM_MANUF:
			flashCmdSet(pFlash, AMD_CHIP_ADDR_ERASE1, 0, AMD_CHIP_CMD_ERASE1);
			flashCmdSet(pFlash, AMD_CHIP_ADDR_ERASE2, 0, AMD_CHIP_CMD_ERASE2);
			flashCmdSet(pFlash, AMD_CHIP_ADDR_ERASE3, 0, AMD_CHIP_CMD_ERASE3);
			flashCmdSet(pFlash, 0, secNum, AMD_CHIP_CMD_ERASE4);
			break;
		case SST_MANUF:
			flashCmdSet(pFlash, SST_CHIP_ADDR_ERASE1, 0, AMD_CHIP_CMD_ERASE1);
			flashCmdSet(pFlash, SST_CHIP_ADDR_ERASE2, 0, AMD_CHIP_CMD_ERASE2);
			flashCmdSet(pFlash, SST_CHIP_ADDR_ERASE3, 0, AMD_CHIP_CMD_ERASE3);
			flashCmdSet(pFlash, 0, secNum, AMD_CHIP_CMD_ERASE4);
			break;

	}

    
    
    /* wait for erase to complete */
    for(i = 0; i < AMD_EARASE_MILI_TIMEOUT; i++)
    {
        mvOsDelay(1);
        if(MV_TRUE == amdFlashStsIsRdy(pFlash, mvFlashSecOffsGet(pFlash,secNum) 
                         + mvFlashBaseAddrGet(pFlash), AMD_CHIP_STAT_DQ7_MASK))
        {
            DB(mvOsPrintf("Flash: amdFlashSecErase erase PASS !!\n"));
            return MV_OK;
        }
    }
    mvOsPrintf("Flash: ERROR amdFlashSecErase timeout \n");	
    
    return MV_TIMEOUT;
}


/*******************************************************************************
* amdFlashProg - Prog busWidth Bits into the address offest in the flash.
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
*	MV_TIMEOUT otherwise.
*
*******************************************************************************/
MV_STATUS amdFlashProg(MV_FLASH_INFO *pFlash,MV_U32 offset, MV_U32 data)
{
    MV_U32 i;
	MV_U32 statusExpectedData;
    
    DB(mvOsPrintf("Flash: amdFlashProg offset %x data %x\n",offset,data));
    
    /* write sequence */
    amdFlashUnlock(pFlash);

	switch (pFlash->flashSpec.flashVen)
	{
		case AMD_MANUF:
		case STM_MANUF:
			flashCmdSet(pFlash, AMD_CHIP_ADDR_PROG, 0, AMD_CHIP_CMD_PROG);
			break;
		case SST_MANUF:
			flashCmdSet(pFlash, SST_CHIP_ADDR_PROG, 0, AMD_CHIP_CMD_PROG);
			break;

	}
    flashBusWidthDataWr(pFlash, offset + mvFlashBaseAddrGet(pFlash), data);
    
	/* To check status we need to be sure that the data is in the same endianess
	of the status, the status is always little endian and the data is big endian
	, so we will convert the data to little endian before calling the amdFlashStsIsRdy
	function*/

	switch(pFlash->busWidth)
    {
	case 1: 
			statusExpectedData = data;
            break;
	   case 2:
			statusExpectedData = MV_16BIT_LE(data);
			break;
		case 4:
			statusExpectedData = MV_32BIT_LE(data);
			break;
		default:
			mvOsPrintf("%s ERROR: Bus Width %d Bytes isn't supported.\n",
                        __FUNCTION__, pFlash->busWidth);
			return MV_TIMEOUT;
	}

	
    /* wait for write to complete */
    for(i = 0; i < AMD_PROG_TIMEOUT; i++)
    {
        if(MV_TRUE == amdFlashStsIsRdy(pFlash,offset + 
                                              mvFlashBaseAddrGet(pFlash),
											  statusExpectedData))
        {
            DB(mvOsPrintf("Flash: amdFlashProg prog PASS !!\n"));
            return MV_OK;
        }
    }
    
    mvOsPrintf("Flash: ERROR amdFlashSecErase timeout \n");	
    
    return MV_TIMEOUT;
}

/*******************************************************************************
* There are few ways to check if the AMD flash is busy or not:		
* 1) by checking DQ7 [and 5 - optional ]				
* 2) by checking toggle bit DQ7 (linux implementation)				
* here we used the first option:
* after write/erase the flash push not(DQ7) until the opertion is completed.
*******************************************************************************/
static MV_BOOL amdFlashStsIsRdy(MV_FLASH_INFO *pFlash, MV_U32 addr, 
                                                                MV_U32 expected)
{
    MV_U32  status;
    
    status = flashBusWidthRd(pFlash, addr);
    /* if DQ7 == Datum 7 */
    if((status & flashDataExt(pFlash, AMD_CHIP_STAT_DQ7_MASK )) == 
        (flashDataExt(pFlash,expected) & 
         flashDataExt(pFlash, AMD_CHIP_STAT_DQ7_MASK)))
    {
        DB(mvOsPrintf("Flash: amdFlashStatusChk value is ready \n"));
        return MV_TRUE;
    }
    DB(mvOsPrintf("Flash: amdFlashStatusChk staus not ready \n")); 
    return MV_FALSE;
}

/*******************************************************************************
* Sequence for Unlocking the flash before accessing the flash.
*******************************************************************************/
static MV_VOID amdFlashUnlock(MV_FLASH_INFO *pFlash)
{
	switch (pFlash->flashSpec.flashVen)
	{
		case AMD_MANUF:
		case STM_MANUF:
			flashCmdSet(pFlash, AMD_CHIP_UNLOCK_ADDR1, 0, AMD_CHIP_UNLOCK_CMD1);
			flashCmdSet(pFlash, AMD_CHIP_UNLOCK_ADDR2, 0, AMD_CHIP_UNLOCK_CMD2);
			break;
		case SST_MANUF:
			flashCmdSet(pFlash, SST_CHIP_UNLOCK_ADDR1, 0, AMD_CHIP_UNLOCK_CMD1);
			flashCmdSet(pFlash, SST_CHIP_UNLOCK_ADDR2, 0, AMD_CHIP_UNLOCK_CMD2);
			break;		
	}
    return;
}



