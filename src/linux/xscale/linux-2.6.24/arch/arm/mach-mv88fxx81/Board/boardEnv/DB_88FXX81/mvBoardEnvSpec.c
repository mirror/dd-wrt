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


#include "mvBoardEnvLib.h"
#include "mvCtrlEnvSpec.h"
#include "mvCpuIf.h"
#include "mvCntmr.h"
#include "mvDevice.h"
#include "mvDramIfRegs.h"
#include "mvPciRegs.h"
#include "mvPexRegs.h"
#include "mvTwsi.h"


/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	

MV_STATUS boardEepromGet(BOARD_DATA    *boardData);

MV_U32 gBoardId = -1;
/*******************************************************************************
* mvBoardIdGet - Get Board model
*
* DESCRIPTION:
*       This function returns board ID.
*       Board ID is 32bit word constructed of board model (16bit) and 
*       board revision (16bit) in the following way: 0xMMMMRRRR.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit board ID number, '-1' if board is undefined.
*
*******************************************************************************/
MV_U32 mvBoardIdGet(MV_VOID)
{
	MV_U32 tmpBoardId = -1;
	BOARD_DATA    boardData;

	if(gBoardId != -1)
		return gBoardId;

#if defined(MV_88F1181)
	
	if(boardEepromGet(&boardData) == MV_OK)
	{
		tmpBoardId = (MV_U32)boardData.boardId;
	}
	else
	{
		/* until we have relevant data in twsi then we 
		will detect the board type from sdram config reg */
		if (MV_REG_READ(SDRAM_CONFIG_REG) & SDRAM_DTYPE_DDR2)
		{
			tmpBoardId = DB_88F1181_DDR2;
		}
		else
		{
			tmpBoardId = DB_88F1181_DDR1;
		}

	}
	

#elif defined(MV_88F5181)

	if (MV_REG_READ(SDRAM_CONFIG_REG) & SDRAM_DTYPE_DDR2)
	{
	}
	else /* DDR1 */
	{
		#if defined(RD_88F5182)
		tmpBoardId = RD_88F5182_2XSATA;
		#elif defined(RD_88F5182_3)
		tmpBoardId = RD_88F5182_2XSATA3;
		#elif defined(RD_88W8660)
		tmpBoardId = RD_88W8660_DDR1;
		#elif defined(RD_88F5181L_FE)
		tmpBoardId = RD_88F5181L_VOIP_FE;
		#elif defined(RD_88F5181L_GE)
		tmpBoardId = RD_88F5181L_VOIP_GE;
        	#elif defined(MV_POS_NAS) 
        	tmpBoardId = RD_88F5181_POS_NAS;
		#elif defined(MV_VOIP)
		tmpBoardId = RD_88F5181_VOIP;
        	#elif defined(DB_PRPMC)
		tmpBoardId = DB_88F5181_DDR1_PRPMC;
		#elif defined(DB_PEX_PCI)
		tmpBoardId = DB_88F5181_DDR1_PEXPCI;
		#endif
	}
	if(tmpBoardId != -1) {
		gBoardId = tmpBoardId;
		return tmpBoardId;
	}


	if(boardEepromGet(&boardData) == MV_OK)
	{
		tmpBoardId = (MV_U32)boardData.boardId;
	}
	else
	{
		/* until we have relevant data in twsi then we 
		will detect the board type from sdram config reg */
		if (MV_REG_READ(SDRAM_CONFIG_REG) & SDRAM_DTYPE_DDR2)
		{
			if((mvCtrlModelGet() == MV_5281_DEV_ID)&&
			   (mvCtrlRevGet() >= MV_5281_B0_REV))
			{
				tmpBoardId = DB_88F5X81_DDR2;
			} 
			else if(mvCtrlModelGet() == MV_8660_DEV_ID)
			{
				tmpBoardId = DB_88W8660_DDR2;
			}
			else if(mvCtrlModelGet() == MV_5182_DEV_ID)
			{
				tmpBoardId = DB_88F5182_DDR2;
			}
			else
			{
				tmpBoardId = DB_88F5181_5281_DDR2;
			}
		}
		else /* DDR1 */
		{
			if (MV_REG_READ(PCI_ARBITER_CTRL_REG(0)) & PACR_ARB_ENABLE) /* arbiter enabled*/
			{
				if((mvCtrlModelGet() == MV_5281_DEV_ID)&&
				   (mvCtrlRevGet() >= MV_5281_B0_REV))
				{
					tmpBoardId = DB_88F5X81_DDR1;
				}
				else
				{
					tmpBoardId = DB_88F5181_5281_DDR1;
				}
			}
			else /* arbiter disabled */
			{

				if ((MV_REG_READ(PEX_CTRL_REG(0)) & PXCR_DEV_TYPE_CTRL_MASK)
					 == PXCR_DEV_TYPE_CTRL_CMPLX) /*root complex*/
				{
					tmpBoardId = DB_88F5181_DDR1_PRPMC;
				}
				else if ((MV_REG_READ(PEX_CTRL_REG(0)) & PXCR_DEV_TYPE_CTRL_MASK)
					 == PXCR_DEV_TYPE_CTRL_POINT) /*end point*/
				{
					tmpBoardId = DB_88F5181_DDR1_PEXPCI;
				}
			}
		}

	}

	gBoardId = tmpBoardId;


	return tmpBoardId;

#else
#   error "CHIP not selected"
#endif



}

/*******************************************************************************
* boardEepromGet - Get board identification from the EEPROM
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*
*******************************************************************************/
MV_STATUS boardEepromGet(BOARD_DATA    *boardData)
{
 	MV_TWSI_SLAVE twsiSlave;
        MV_TWSI_ADDR slave;

	MV_U32 tclk;

	tclk = mvBoardTclkGet();
	
	/* Init TWSI first */
	slave.type = ADDR7_BIT;
	slave.address = 0x0;
	mvTwsiInit(100000, tclk, &slave, 0);

   	twsiSlave.slaveAddr.address = MV_BOARD_ID_EEPROM;
	twsiSlave.slaveAddr.type = ADDR7_BIT;
	twsiSlave.validOffset = MV_TRUE;
   	twsiSlave.offset = MV_BOARD_ID_EEPROM_OFFSET0;
	twsiSlave.moreThen256 = MV_FALSE;

	if(MV_OK != mvTwsiRead (&twsiSlave, (MV_U8*)boardData, sizeof(BOARD_DATA)))
    {
		/*mvOsOutput("Fail to read Board EEPROM from offset0");*/
       	return MV_FAIL;
   	}

#if defined(MV_CPU_LE)
	boardData->magic = MV_BYTE_SWAP_32BIT(boardData->magic);
	boardData->boardId = MV_BYTE_SWAP_16BIT(boardData->boardId);
	boardData->reserved1 = MV_BYTE_SWAP_32BIT(boardData->reserved1);
	boardData->reserved2 = MV_BYTE_SWAP_32BIT(boardData->reserved2);
#endif

	if(boardData->magic == MV_BOARD_I2C_MAGIC)
	{
    	return MV_OK;
    }

   	twsiSlave.offset = MV_BOARD_ID_EEPROM_OFFSET1;
	twsiSlave.moreThen256 = MV_TRUE;

	if(MV_OK != mvTwsiRead (&twsiSlave, (MV_U8*)boardData, sizeof(BOARD_DATA)))
    {
		/*mvOsOutput("Fail to read Board EEPROM from offset1");*/
       	return MV_FAIL;
   	}


	
#if defined(MV_CPU_LE)
	boardData->magic = MV_BYTE_SWAP_32BIT(boardData->magic);
	boardData->boardId = MV_BYTE_SWAP_16BIT(boardData->boardId);
	boardData->reserved1 = MV_BYTE_SWAP_32BIT(boardData->reserved1);
	boardData->reserved2 = MV_BYTE_SWAP_32BIT(boardData->reserved2);
#endif
	
    if(boardData->magic == MV_BOARD_I2C_MAGIC)
	{
    	return MV_OK;
    }

	return MV_FAIL;
}



