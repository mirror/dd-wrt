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


#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cntmr/mvCntmr.h"
#include "device/mvDevice.h"
#include "ddr1_2/mvDramIfRegs.h"
#include "twsi/mvTwsi.h"


/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif

MV_STATUS boardEepromGet(BOARD_DATA    *boardData);

MV_U32 gBoardId = -1;

#ifdef MV_INCLUDE_EARLY_PRINTK
extern void mv_early_printk(char *fmt,...);
#endif
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

#if defined(DB_FPGA)
	tmpBoardId = DB_88F5X8X_FPGA_DDR1;
	gBoardId = tmpBoardId;
	return tmpBoardId;
#endif

	if(gBoardId != -1)
        {
	    #if defined(MV_88F5182)
            switch(gBoardId)
            {
                case DB_88F5182_DDR2_OLD:
                    gBoardId = DB_88F5182_DDR2;
                    break;
                case RD_88F5182_2XSATA_OLD:
                    gBoardId = RD_88F5182_2XSATA;
                    break;
                case RD_88F5182_2XSATA3_OLD:
                    gBoardId = RD_88F5182_2XSATA3;
                    break;
                default:
                break;
	    }
            #elif defined(MV_88F5082)
            switch(gBoardId)
            {
                case DB_88F5082_DDR2_OLD:
                    gBoardId = DB_88F5082_DDR2;
                    break;
		case RD_88F5082_2XSATA_OLD:
                    gBoardId = RD_88F5082_2XSATA;
                    break;
		case RD_88F5082_2XSATA3_OLD:
                    gBoardId = RD_88F5082_2XSATA3;
                    break;
                default:
                    break;
            }
	    #elif defined(MV_88F5181L)
	    if (gBoardId == DB_88F5181L_DDR2_2XTDM_OLD)
	    {
		gBoardId = DB_88F5181L_DDR2_2XTDM;
	    }
	    else if(gBoardId == RD_88F5181L_VOIP_FE_OLD)
	    {
		gBoardId = RD_88F5181L_VOIP_FE;
	    }
	    else if(gBoardId == RD_88F5181L_VOIP_GE_OLD)
	    {
		gBoardId = RD_88F5181L_VOIP_GE;
	    }
	    #elif defined(MV_88W8660)
	    if (gBoardId == DB_88W8660_DDR2_OLD)
	    {
	    	gBoardId = DB_88W8660_DDR2;
	    }
 	    else if (gBoardId == RD_88W8660_DDR1_OLD)
	    {
	    	gBoardId = RD_88W8660_DDR1;
	    }
	    else if(gBoardId == RD_88W8660_AP82S_DDR1_OLD)
	    {
		gBoardId = RD_88W8660_AP82S_DDR1;
	    }
	    #elif defined(MV_88F5181)
	    if (gBoardId == RD_88F5181_GTW_FE_OLD)
	    {
	    	gBoardId = RD_88F5181_GTW_FE;
	    }
 	    else if (gBoardId == RD_88F5181_GTW_GE_OLD)
	    {
	    	gBoardId = RD_88F5181_GTW_GE;
	    }
	    else if (gBoardId == RD_88F5181_POS_NAS_OLD)
	    {
	    	gBoardId = RD_88F5181_POS_NAS;
	    }
            else if (gBoardId == DB_88F5X81_DDR2_OLD)
	    {
	    	gBoardId = DB_88F5X81_DDR2;
	    }
	    else if (gBoardId == DB_88F5X81_DDR1_OLD)
	    {
	    	gBoardId = DB_88F5X81_DDR1;
	    }
	    else if (gBoardId == 0) /* In case that the U-Boot did not pass the baord ID like in DB-88F5181-DDR1-BP */
    	    {
		gBoardId = -1;
	    }
            #endif
	    if(gBoardId != -1)
		return gBoardId;
        }

	if (MV_REG_READ(SDRAM_CONFIG_REG) & SDRAM_DTYPE_DDR2)
	{
		#if defined(DB_88F1281)
		tmpBoardId = DB_88F1281_DDR2;
		#elif defined(DB_88F6183BP)
		tmpBoardId = DB_88F6183_BP;
		#elif defined(RD_88F6183GP)
		tmpBoardId = RD_88F6183_GP;
		#elif defined(RD_88F6183AP)
		tmpBoardId = RD_88F6183_AP;
		#elif defined(DB_88F5181L)
		tmpBoardId = DB_88F5181L_DDR2_2XTDM;
		#elif defined(DB_88W8660)
		tmpBoardId = DB_88W8660_DDR2;
		#elif defined(DB_88F5082)
		tmpBoardId = DB_88F5082_DDR2;
		#elif defined(DB_88F5182)
		tmpBoardId = DB_88F5182_DDR2;
		#elif defined(DB_88F5182_A)
		tmpBoardId = DB_88F5182_DDR2_A;
		#elif defined(DB_88F5181)
		tmpBoardId = DB_88F5X81_DDR2;
		#elif defined(DB_88F5181_OLD)
		tmpBoardId = DB_88F5181_5281_DDR2;
		#elif defined(DB_FPGA)
		tmpBoardId = DB_88F5X8X_FPGA_DDR1;
		#elif defined(DB_88F6082LBP)
		tmpBoardId = DB_88F6082L_BP;
		#elif defined(DB_CUSTOMER)
		tmpBoardId = DB_CUSTOMER1_ID;
		#endif
	}
	else /* DDR1 */
	{
		#if defined(RD_88F5182)
		tmpBoardId = RD_88F5182_2XSATA;
		#elif defined(RD_88F5182_3)
		tmpBoardId = RD_88F5182_2XSATA3;
		#elif defined(RD_88F5082)
		tmpBoardId = RD_88F5082_2XSATA;
		#elif defined(RD_88F5082_3)
		tmpBoardId = RD_88F5082_2XSATA3;
		#elif defined(RD_88W8660)
		tmpBoardId = RD_88W8660_DDR1;
		#elif defined(RD_88W8660_AP82S)
		tmpBoardId = RD_88W8660_AP82S_DDR1;
		#elif defined(RD_88F5181L_FE)
		tmpBoardId = RD_88F5181L_VOIP_FE;
		#elif defined(RD_88F5181L_GE)
		tmpBoardId = RD_88F5181L_VOIP_GE;
		#elif defined(RD_88F5181_GTWGE)
		tmpBoardId = RD_88F5181_GTW_GE;
		#elif defined(RD_88F5181_GTWFE)
		tmpBoardId = RD_88F5181_GTW_FE;
		#elif defined(RD_88F5181L_FXO_GE)
		tmpBoardId = RD_88F5181L_VOIP_FXO_GE;
		#elif defined(MV_POS_NAS)
		tmpBoardId = RD_88F5181_POS_NAS;
		#elif defined(MV_VOIP)
		tmpBoardId = RD_88F5181_VOIP;
		#elif defined(DB_PRPMC)
		tmpBoardId = DB_88F5181_DDR1_PRPMC;
		#elif defined(DB_MNG)
		tmpBoardId = DB_88F5181_DDR1_MNG;
		#elif defined(DB_PEX_PCI)
		tmpBoardId = DB_88F5181_DDR1_PEXPCI;
		#elif defined(DB_88F6082BP)
		tmpBoardId = DB_88F6082_BP;
		#elif defined(DB_88F6082LBP)
		tmpBoardId = DB_88F6082L_BP;
		#elif defined(DB_88F6082SA)
		tmpBoardId = DB_88F6082_SA;
		#elif defined(RD_88F6082NAS)
		tmpBoardId = RD_88F6082_NAS;
		#elif defined(RD_88F6082DAS_PLUS)
		tmpBoardId = RD_88F6082_DAS_PLUS;
		#elif defined(RD_88F6082MICRO_DAS_NAS)
		tmpBoardId = RD_88F6082_MICRO_DAS_NAS;
		#elif defined(RD_88F6082GE_SATA)
		tmpBoardId = RD_88F6082_GE_SATA;
		#elif defined(RD_88F6082_DX243)
		tmpBoardId = RD_88F6082_DX243_24G;
		#elif defined(DB_88F5181)
		tmpBoardId = DB_88F5X81_DDR1;
		#elif defined(DB_88F5181_OLD)
		tmpBoardId = DB_88F5181_5281_DDR1;
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
			else if(mvCtrlModelGet() == MV_5082_DEV_ID)
			{
				tmpBoardId = DB_88F5082_DDR2;
			}
			else if(mvCtrlModelGet() == MV_5182_DEV_ID)
			{
				tmpBoardId = DB_88F5182_DDR2;
			}
			else if(mvCtrlModelGet() == MV_5180_DEV_ID)
			{
				tmpBoardId = DB_88F5180N_DDR2;
			}
            else if(mvCtrlModelGet() == MV_6183_DEV_ID)
            {
                tmpBoardId = DB_88F6183_BP;
            }
			else
			{
				tmpBoardId = DB_88F5181_5281_DDR2;
			}
		}
		else /* DDR1 */
		{
			if((mvCtrlModelGet() == MV_5281_DEV_ID)&&
			   (mvCtrlRevGet() >= MV_5281_B0_REV))
			{
				tmpBoardId = DB_88F5X81_DDR1;
			}
			else if(mvCtrlModelGet() == MV_5180_DEV_ID)
			{
				tmpBoardId = DB_88F5180N_DDR1;
			}
			else
			{
				tmpBoardId = DB_88F5181_5281_DDR1;
			}
		}

	}

	gBoardId = tmpBoardId;

#ifdef MV_INCLUDE_EARLY_PRINTK
	if (tmpBoardId == -1)
	{
		mv_early_printk("%s FATAL ERROR: Failed to detect Board ID\n", __FUNCTION__);
	}
#endif

	return tmpBoardId;


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
		/* Backward compatability */
		#if defined(MV_88F5182)
		if (boardData->boardId == DB_88F5182_DDR2_OLD)
		{
			boardData->boardId = DB_88F5182_DDR2;
		}
		#elif defined(MV_88F5181L)
		if (boardData->boardId == DB_88F5181L_DDR2_2XTDM_OLD)
		{
			boardData->boardId = DB_88F5181L_DDR2_2XTDM;
		}
		else if(boardData->boardId == RD_88F5181L_VOIP_FE_OLD)
	        {
			boardData->boardId = RD_88F5181L_VOIP_FE;
	        }

		#elif defined(MV_88W8660)
		if (boardData->boardId == DB_88W8660_DDR2_OLD)
		{
			boardData->boardId = DB_88W8660_DDR2;
		}
		#endif
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
		/* Backward compatability */
		/* Backward compatability */
		#if defined(MV_88F5182)
		if (boardData->boardId == DB_88F5182_DDR2_OLD)
		{
			boardData->boardId = DB_88F5182_DDR2;
		}
		#elif defined(MV_88F5181L)
		if (boardData->boardId == DB_88F5181L_DDR2_2XTDM_OLD)
		{
			boardData->boardId = DB_88F5181L_DDR2_2XTDM;
		}
		else if(boardData->boardId == RD_88F5181L_VOIP_FE_OLD)
	        {
			boardData->boardId = RD_88F5181L_VOIP_FE;
	        }

		#elif defined(MV_88W8660)
		if (boardData->boardId == DB_88W8660_DDR2_OLD)
		{
			boardData->boardId = DB_88W8660_DDR2;
		}
		#endif
    	return MV_OK;
    }

	return MV_FAIL;
}



