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
#include "mvGpp.h"
#include "mvPciIf.h"

/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	



#define CODE_IN_ROM				MV_FALSE
#define CODE_IN_RAM				MV_TRUE



#if defined(MV_88F5182)


MV_U32	boardIdToIndex[MV_MAX_BOARD_ID]	={	N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											0,
											1,
											N_A,
											N_A,
											N_A,
											2
											};

MV_BOARD_INFO	mv88F5182InfoTbl[3]	={DB_88F5182_DDR2_INFO,
									  RD_88F5182_2XSATA_INFO,
									  RD_88F5182_2XSATA3_INFO
									  };


#elif defined(RD_DB_88F5181L)

MV_U32	boardIdToIndex[MV_MAX_BOARD_ID]	={	N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											N_A,
											0,
											1,
											2
											};

MV_BOARD_INFO	mv88F5181LInfoTbl[3]={DB_88F5181L_DDR2_2XTDM_INFO,
									  RD_88F5181L_VOIP_FE_INFO,
									  RD_88F5181L_VOIP_GE_INFO
									  };
#elif defined(MV_88W8660)

MV_U32	boardIdToIndex[MV_MAX_BOARD_ID]	={ N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A,
					   N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A,
					     0,   1};
											

MV_BOARD_INFO	mv88w8660InfoTbl[3]= { DB_88W8660_DDR2_INFO, RD_88W8660_DDR1_INFO };

#else

MV_U32	boardIdToIndex[MV_MAX_BOARD_ID]		={	DB_88F1181_DDR1,
												DB_88F1181_DDR2,
												DB_88F5181_5281_DDR1,
												DB_88F5181_5281_DDR2,
												DB_88F5181_DDR1_PRPMC,
												DB_88F5181_DDR1_PEXPCI,
												RD_88F5181_POS_NAS,
												DB_88F5X81_DDR2,
												DB_88F5X81_DDR1,
												RD_88F5181_VOIP,
												N_A,
												N_A,
												N_A,
												N_A
												};


MV_BOARD_INFO	defBoardInfoTbl[10]	={	DB_88F1181_DDR1_INFO,
									DB_88F1181_DDR2_INFO,
									DB_88F5181_5281_DDR1_INFO,
									DB_88F5181_5281_DDR2_INFO,
									DB_88F5181_DDR1_PRPMC_INFO,
									DB_88F5181_DDR1_PEXPCI_INFO,
									RD_88F5181_POS_NAS_INFO,
									DB_88F5X81_DDR2_INFO,
									DB_88F5X81_DDR1_INFO,
									RD_88F5181_VOIP_INFO
									};


#endif


#if defined(MV_88F5182)
#define MV_CURRENT_TABLE_INFO	mv88F5182InfoTbl

#elif defined(RD_DB_88F5181L)
	  
#define MV_CURRENT_TABLE_INFO	mv88F5181LInfoTbl

#elif defined(MV_88W8660)

#define MV_CURRENT_TABLE_INFO  mv88w8660InfoTbl

#else

#define MV_CURRENT_TABLE_INFO	defBoardInfoTbl
#endif

#define BOARD_INFO  boardInfoTbl[boardIdToIndex[boardId]] 


/* Locals */
MV_VOID   refClkInit(MV_VOID);
MV_VOID   refClkWaitForLow(MV_VOID);
MV_VOID   refClkWaitNCycles(MV_U32 cycles);
MV_U32	  boardGetDevCSNum(MV_32 devNum, MV_BOARD_DEV_TYPE devType);


/*******************************************************************************
* mvBoardEnvInit - Init board
*
* DESCRIPTION:
*		In this function the board environment take care of device bank 
*		initialization.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardEnvInit(MV_VOID)
{
	MV_U32 devNum;
	MV_U32 devBankParam=0;
	MV_U32 boardId= mvBoardIdGet();
	MV_U32 deviceId= mvCtrlModelGet();
	MV_BOARD_INFO	*boardInfoTbl=NULL;
	MV_U32	regVal;
	int i;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardEnvInit:Board unknown.\n");
		return;

	}

    /* set GPP Out enable for debug Leds. Check if Debug leds used via GPP  */
	if (BOARD_INFO.ledGppPin[0] != (MV_U8)N_A)
	{
		for (i = 0; i < BOARD_INFO.activeLedsNumber; i++)
		{
			mvGppOutEnablle(0, 1 << BOARD_INFO.ledGppPin[i], 0);
		}
	}

	if(DB_88F5181_5281_DDR2 == boardId)
	{
		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFFFFF, 0xFFFF3717);

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0x3002);
	}
	else if(DB_88F5181_5281_DDR1 == boardId)
	{
		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0xc0);
	}
	else if((DB_88F5X81_DDR2 == boardId)||
		(DB_88F5X81_DDR1 == boardId))
	{
		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFFFFF, 0xFFFDFF17);

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0x3400);
	}
	else if (DB_88F5182_DDR2 == boardId)
	{
		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFFFFF, 0xFFF5FFD7);

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0x403);

	}
	else if (RD_88F5182_2XSATA == boardId)
	{
		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFFFFF, 0xFFF0F0CA);

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0xd0);

	}
	else if (RD_88F5182_2XSATA3 == boardId)
	{

		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFFFFF, 0xFFF0F04A);

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0x50);

		/* Set High GPP 2 */
		mvGppValueSet (0, (1 << 2), (1 << 2));

	}
    else if(DB_88F5181_DDR1_PRPMC == boardId)
	{
		/* check MONARCHn*/
		if (MV_GPP2 == mvGppValueGet(0,MV_GPP2))
		{
			/* set GPP Out enable */
			MV_REG_WRITE(0x10104,0xFFFF8F4C);

			/* set GPP polarity */
			MV_REG_WRITE(0x1010C,0x0000000C);
            
		}
		else
		{
			/* set GPP Out enable */
			MV_REG_WRITE(0x10104,0xFFFF8FFC);

			/* set GPP polarity */
			MV_REG_WRITE(0x1010C,0x000000FC);

		}
	}
	else if (DB_88F5181_DDR1_PEXPCI == boardId)
	{

		/* set GPP Out enable */
		mvGppOutEnablle(0,MV_GPP0|MV_GPP12|MV_GPP13|MV_GPP14|MV_GPP15,0);

		/* Pex reset out*/
		mvGppValueSet(0,MV_GPP0,1);

	}
	else if (DB_88F5181L_DDR2_2XTDM == boardId)
	{
		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFFFFF, 0xFFFDFFD7);

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0x3700);

	}
	else if (RD_88F5181L_VOIP_FE == boardId)
	{
		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFFFFF, 0xFFFF0FFC);

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0xA3C);

	}
	else if (RD_88F5181L_VOIP_GE == boardId)
	{        
		/* Set GPP Out Enable*/      
		mvGppOutEnablle(0, 0xFFFFFFFF, 0xFFFF07F0);
        
#ifdef CONFIG_SCM_SUPPORT
        mvGppOutEnablle(0, MV_GPP6|MV_GPP7,
                        (MV_GPP_OUT&MV_GPP6)|(MV_GPP_OUT&MV_GPP7));
        /* turn off usb and wireless led */
        mvGppValueSet(0,MV_GPP6|MV_GPP7,MV_GPP6|MV_GPP7);
#endif        

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFFFFF, 0x530);

	}
	else if (DB_88W8660_DDR2 == boardId)
	{
		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFF, 0xFFF5); 

		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFF, 0xE00); 

	}
	else if (RD_88W8660_DDR1 == boardId)
	{
		/* Set GPP Out Enable*/
		mvGppOutEnablle(0, 0xFFFFF, 0xFFF1C);
 
		/* set GPP polarity */
		mvGppPolaritySet(0, 0xFFFFF, 0xA18); 

		/* Clock Run */
                mvGppValueSet(0, MV_GPP1, MV_GPP1);

	}

	for (devNum = START_DEV_CS; devNum < MV_DEV_MAX_CS; devNum++)
	{
		devBankParam = BOARD_INFO.devCsInfo[devNum].params;

		if (devBankParam == N_A) continue;

		if (devNum != BOOT_CS)
		{
			MV_REG_WRITE(DEV_BANK_PARAM_REG(devNum), devBankParam);
		}
		else
		{
			MV_U32 bootDevBankParam;

			/* for BootCS Only device width should be as in sample at
			reset */
			bootDevBankParam = MV_REG_READ(DEV_BOOT_BANK_PARAM_REG);
			bootDevBankParam &= DBP_DEVWIDTH_MASK;
			devBankParam &= ~DBP_DEVWIDTH_MASK;
			devBankParam |= bootDevBankParam;

			MV_REG_WRITE(DEV_BOOT_BANK_PARAM_REG , devBankParam);
		}
	}

	/* NAND flash stuff */
    if((deviceId == MV_5281_DEV_ID) ||                            /* Orion2   */
	   (deviceId == MV_5182_DEV_ID) ||                            /* OrionNAS */
	   (deviceId == MV_8660_DEV_ID) ||                            /* 88w8660  */
      ((deviceId == MV_5181_DEV_ID) && (mvCtrlRevGet() >= 0x8)))  /* Orion1L  */
	{

        /* If we are booting from NAND MPPs should be modified */
        /* Check NAND connected to boot device */
        if (MV_REG_READ(DEV_NAND_CTRL_REG) & 0x1)
        {           
            boardInfoTbl[boardIdToIndex[boardId]].mppGroup[0] &= 0xFF00FFFF;
            boardInfoTbl[boardIdToIndex[boardId]].mppGroup[0] |= 0x00440000;
            boardInfoTbl[boardIdToIndex[boardId]].mppGroup[1] &= 0x00FFFFFF;            
        }
		else
        {            
			if (boardGetDevCSNum(0, BOARD_DEV_NAND_FLASH) != 0xffffffff)
			{
				/* We always use care mode. */
				mvDevNandSet(boardGetDevCSNum(0, BOARD_DEV_NAND_FLASH), 1);
			}
        }
	}

#ifndef MV_88W8660
	/* Guideline (GL# ETH-3) RGMII Output Delay Tuning*/

	/* Read if we are in RGMII mode */
	regVal = MV_REG_READ(MPP_SAMPLE_AT_RESET);

	/* Check if we are in RGMII mode */
	if (MSAR_GIGA_PORT_MODE_RGMII == (regVal & MSAR_GIGA_PORT_MODE_MASK))
	{

		regVal = MV_REG_READ(DEV_RGMII_AC_TIMING_REG);
		regVal &= ~0x3;
		regVal |= 0x2;
		MV_REG_WRITE(DEV_RGMII_AC_TIMING_REG, regVal);
	}
#endif

}

/* maen : all board ID functions should be updated */
/*******************************************************************************
* mvBoardModelGet - Get Board model
*
* DESCRIPTION:
*       This function returns 16bit describing board model.
*       Board model is constructed of one byte major and minor numbers in the 
*       following manner:
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       String describing board model.
*
*******************************************************************************/
MV_U16 mvBoardModelGet(MV_VOID)
{
	return (mvBoardIdGet() >> 16);
}

/*******************************************************************************
* mbBoardRevlGet - Get Board revision
*
* DESCRIPTION:
*       This function returns a 32bit describing the board revision.
*       Board revision is constructed of 4bytes. 2bytes describes major number
*       and the other 2bytes describes minor munber. 
*       For example for board revision 3.4 the function will return 
*       0x00030004.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       String describing board model.
*
*******************************************************************************/
MV_U16 mvBoardRevGet(MV_VOID)
{
	return (mvBoardIdGet() & 0xFFFF);
}

/*******************************************************************************
* mvBoardNameGet - Get Board name
*
* DESCRIPTION:
*       This function returns a string describing the board model and revision.
*       String is extracted from board I2C EEPROM.
*
* INPUT:
*       None.
*
* OUTPUT:
*       pNameBuff - Buffer to contain board name string. Minimum size 32 chars.
*
* RETURN:
*       
*       MV_ERROR if informantion can not be read.
*******************************************************************************/
MV_STATUS mvBoardNameGet(char *pNameBuff)
{
	MV_U32 boardId= mvBoardIdGet();
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsSPrintf (pNameBuff, "Board unknown.\n");
		return MV_ERROR;

	}

	mvOsSPrintf (pNameBuff, "%s",BOARD_INFO.boardName);

	return MV_OK;
}

/*******************************************************************************
* mvBoardPhyAddrGet - Get the phy address
*
* DESCRIPTION:
*       This routine returns the Phy address of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing Phy address, '-1' if the port number is wrong.
*
*******************************************************************************/
MV_U32 mvBoardPhyAddrGet(MV_U32 ethPortNum)
{
	MV_U32 boardId= mvBoardIdGet();
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardPhyAddrGet:Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO.ethPhyAddr[ethPortNum];
}

/*******************************************************************************
* mvBoardTclkGet - Get the board Tclk (Controller clock)
*
* DESCRIPTION:
*       This routine extract the controller core clock.
*       This function uses the controller counters to make identification. 
*		Note: In order to avoid interference, make sure task context switch
*		and interrupts will not occure during this function operation
*
* INPUT:
*       countNum - Counter number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32 mvBoardTclkGet(MV_VOID)
{

	MV_U32 	tmpTClkRate=0;

#ifdef TCLK_AUTO_DETECT

	tmpTClkRate = MV_REG_READ(MPP_SAMPLE_AT_RESET);

	tmpTClkRate &= MSAR_TCLCK_MASK;

	switch (tmpTClkRate)
	{
	case MSAR_TCLCK_133:
		tmpTClkRate = 133000000;
		break;
	case MSAR_TCLCK_150:
		tmpTClkRate = 150000000;
		break;
	case MSAR_TCLCK_166:
		tmpTClkRate = 166000000;
		break;
	}

#else
	
	tmpTClkRate = MV_BOARD_DEFAULT_TCLK;

#endif

	return tmpTClkRate;

}
/*******************************************************************************
* mvBoardSysClkGet - Get the board SysClk (CPU bus clock)
*
* DESCRIPTION:
*       This routine extract the CPU bus clock.
*
* INPUT:
*       countNum - Counter number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32  mvBoardSysClkGet(MV_VOID)
{
	MV_U32 	tmpSysClkRate=0;
	MV_U32	tmp;
	
#ifdef SYSCLK_AUTO_DETECT
		
		tmp = MV_REG_READ(MPP_SAMPLE_AT_RESET);

		tmpSysClkRate = tmp & MSAR_ARMDDRCLCK_MASK;
		
		if (mvCtrlModelGet() == MV_5281_DEV_ID)
			if(tmp & MSAR_ARMDDRCLCK_H_MASK) 
				tmpSysClkRate |= BIT8;

		switch (tmpSysClkRate)
		{
		case MSAR_ARMDDRCLCK_333_167:
			tmpSysClkRate = 166000000;
			break;
		case MSAR_ARMDDRCLCK_400_200:
			tmpSysClkRate = 200000000;
			break;
		case MSAR_ARMDDRCLCK_400_133:
			tmpSysClkRate = 133000000;
			break;
		case MSAR_ARMDDRCLCK_500_167:
			tmpSysClkRate = 166000000;
			break;
		case MSAR_ARMDDRCLCK_533_133:
			tmpSysClkRate = 133000000;
			break;
		case MSAR_ARMDDRCLCK_600_200:
			tmpSysClkRate = 200000000;
			break;
		case MSAR_ARMDDRCLCK_667_167:
			tmpSysClkRate = 166000000;
			break;
		case MSAR_ARMDDRCLCK_800_200:	
			tmpSysClkRate = 200000000;
			break;
		case MSAR_ARMDDRCLCK_550_183:
			tmpSysClkRate = 183000000;
			break;
		case MSAR_ARMDDRCLCK_480_160:
			tmpSysClkRate = 160000000;
			break;
		case MSAR_ARMDDRCLCK_466_233:
			tmpSysClkRate = 233000000;
			break;
		case MSAR_ARMDDRCLCK_500_250:
			tmpSysClkRate = 250000000;
			break;
		case MSAR_ARMDDRCLCK_525_175:
			tmpSysClkRate = 175000000;
			break;
		case MSAR_ARMDDRCLCK_533_266:
			tmpSysClkRate = 266000000;
			break;
		case MSAR_ARMDDRCLCK_600_300:
			tmpSysClkRate = 300000000;
			break;
		case MSAR_ARMDDRCLCK_450_150:
			tmpSysClkRate = 150000000;
			break;
		case MSAR_ARMDDRCLCK_533_178:
			tmpSysClkRate = 178000000;
			break;
		case MSAR_ARMDDRCLCK_575_192:
			tmpSysClkRate = 192000000;
			break;
		case MSAR_ARMDDRCLCK_700_175:
			tmpSysClkRate = 175000000;
			break;
		case MSAR_ARMDDRCLCK_733_183:
			tmpSysClkRate = 183000000;
			break;
		case MSAR_ARMDDRCLCK_750_187:
			tmpSysClkRate = 187000000;
			break;
		case MSAR_ARMDDRCLCK_775_194:
			tmpSysClkRate = 194000000;
			break;
		case MSAR_ARMDDRCLCK_500_125:
			tmpSysClkRate = 125000000;
			break;
		case MSAR_ARMDDRCLCK_500_100:
			tmpSysClkRate = 100000000;
			break;
		case MSAR_ARMDDRCLCK_600_150:
			tmpSysClkRate = 150000000;
			break;
		}

#else
		tmpSysClkRate = MV_BOARD_DEFAULT_SYSCLK;
#endif

		return tmpSysClkRate;
}


/*******************************************************************************
* mvBoardPexBridgeIntPinGet - Get PEX to PCI bridge interrupt pin number
*
* DESCRIPTION:
*		Multi-ported PCI Express bridges that is implemented on the board 
*		collapse interrupts across multiple conventional PCI/PCI-X buses.
*		A dual-headed PCI Express bridge would map (or "swizzle") the 
*		interrupts per the following table (in accordance with the respective 
*		logical PCI/PCI-X bridge's Device Number), collapse the INTA#-INTD# 
*		signals from its two logical PCI/PCI-X bridges, collapse the 
*		INTA#-INTD# signals from any internal sources, and convert the 
*		signals to in-band PCI Express messages. 10
*		This function returns the upstream interrupt as it was converted by 
*		the bridge, according to board configuration and the following table:
*					  		PCI dev num		
*			Interrupt pin 	7, 	8, 	9
*		   			A  ->	A	D	C
*		   			B  -> 	B	A	D
*		   			C  -> 	C	B	A
*		  			D  ->	D	C	B
*
*
* INPUT:
*       devNum - PCI/PCIX device number.
*       intPin - PCI Int pin
*
* OUTPUT:
*       None.
*
* RETURN:
*       Int pin connected to the Interrupt controller
*
*******************************************************************************/
MV_U32 mvBoardPexBridgeIntPinGet(MV_U32 devNum, MV_U32 intPin)
{
	MV_U32 realIntPin = ((intPin + (3 - (devNum % 4))) %4 );

	if (realIntPin == 0) return 4;
		else return realIntPin;

}

/*******************************************************************************
* mvBoardDebug7Seg - Set the board debug 7Seg
*
* DESCRIPTION:
*
* INPUT:
*       hexNum - Number to be displied in hex by 7Seg.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardDebug7Seg(MV_U32 hexNum)
{
	MV_U32 	boardId,
			addr,val = 0,totalMask,
			currentBitMask = 1,i;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardDebug7Seg:Board unknown.\n");
		return;

	}

	totalMask = (1 << BOARD_INFO.activeLedsNumber) -1;
	hexNum &= totalMask;
	totalMask = 0;

	/* Check if 7Segments is wired to CS */
	addr = mvBoardGetDeviceBaseAddr(0, BOARD_DEV_SEVEN_SEG);

	if (addr != 0xFFFFFFFF)
	{
		hexNum = *(volatile MV_U32*)((MV_U32)CPU_MEMIO_UNCACHED_ADDR(addr) + 
													(MV_U32)(hexNum << 4));

		return;

	}

	/* the 7seg is wired to GPPs */
	for (i = 0 ; i < BOARD_INFO.activeLedsNumber ; i++)
	{
		if (hexNum & currentBitMask)
		{
			val |= (1 << BOARD_INFO.ledGppPin[i]);
		}

		totalMask |= (1 << BOARD_INFO.ledGppPin[i]);

		currentBitMask = (currentBitMask << 1);
	}

	
	if (BOARD_INFO.ledsPolarity)
	{
		mvGppValueSet(0, totalMask, val);
	}
	else
	{
		mvGppValueSet(0, totalMask, ~val);
	}

	

}

/*******************************************************************************
* mvBoardPciGpioPinGet - Get board PCI interrupt level.
*
* DESCRIPTION:
*		This function returns the value of Gpp Pin that is connected 
*		to the specified IDSEL and interrupt pin (A,B,C,D). For example, If 
*		IDSEL 8 (device 8) interrupt A is connected to GPIO pin 4 the function 
*		will return the value 4.
*		This function supports multiple PCI interfaces.
*
* INPUT:
*		pciIf  - PCI interface number. 
*		devNum - device number (IDSEL).
*		intPin - Interrupt pin (A=1, B=2, C=3, D=4).
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_U32 mvBoardPciGpioPinGet(MV_U32 pciIf, MV_U32 devNum, MV_U32 intPin)
{
	int slotNumber;
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	/* Convert PciIf to the real PCi Interface number */
	pciIf = mvPciRealIfNumGet(pciIf);

	boardId = mvBoardIdGet();
	

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardPciGpioPinGet:Board ID %d unknown.\n", boardId);
		return MV_ERROR;

	}

	if ((DB_88F5181_DDR1_PRPMC  == boardId) || 
        (DB_88F5181_DDR1_PEXPCI == boardId))
	{
        /* These boards are NOT backplans. PCI interrupt connectivity   */
        /* information of a specifc user backplain, which to install    */
        /* those boards, is unknown.                                    */
        /* Marvell general HAL provides default PCI definition for      */
        /* these add-in cards. Each user should modify this             */
        /* configuration according to the backplain in use.             */
        
        return (MV_U32)BOARD_INFO.pciBoardIf[pciIf].pciSlot[0].pciSlotGppIntMap[intPin - 1];
    }
	
    if (BOARD_INFO.pciBoardIf[pciIf].pciSlotsNum == (MV_U8)N_A)
	{
		mvOsPrintf("mvBoardPciGpioPinGet: ERR. Could not find GPP pin " \
                   "assignment for pciIf %d devNum %d intPin %d\n",
                   pciIf, devNum, intPin);
		return N_A;
	}

	if ((devNum <(MV_U32) BOARD_INFO.pciBoardIf[pciIf].firstSlotDevNum )||
		(devNum >= (MV_U32)BOARD_INFO.pciBoardIf[pciIf].firstSlotDevNum +  
			       (MV_U32)BOARD_INFO.pciBoardIf[pciIf].pciSlotsNum))
	{

		mvOsPrintf("mvBoardPciGpioPinGet:Illigal device number %d\n", devNum);
		return N_A;

	}
	slotNumber = devNum - BOARD_INFO.pciBoardIf[pciIf].firstSlotDevNum;

	return (MV_U32)BOARD_INFO.pciBoardIf[pciIf].pciSlot[slotNumber].pciSlotGppIntMap[intPin - 1];

	
}

/*******************************************************************************
* mvBoardRTCGpioPinGet - mvBoardRTCGpioPinGet
*
* DESCRIPTION:
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_U32 mvBoardRTCGpioPinGet(MV_VOID)
{
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardRTCGpioPinGet:Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO.rtcIntPin;
}

/*******************************************************************************
* mvBoardUSBVbusGpioPinGet - return Vbus input GPP
*
* DESCRIPTION:
*
* INPUT:
*		int  devNo.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_U32 mvBoardUSBVbusGpioPinGet(int devId)
{
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardUSBVbusGpioPinGet:Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO.vbusUsbGppPin[devId];
}


/*******************************************************************************
* mvBoardGpioIntMaskGet - Get GPIO mask for interrupt pins
*
* DESCRIPTION:
*		This function returns a 32-bit mask of GPP pins that connected to 
*		interrupt generating sources on board.
*		For example if UART channel A is hardwired to GPP pin 8 and 
*		UART channel B is hardwired to GPP pin 4 the fuinction will return
*		the value 0x000000110
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*		See description. The function return -1 if board is not identified.
*
*******************************************************************************/
MV_U32 mvBoardGpioIntMaskGet(MV_VOID)
{
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardGpioIntMaskGet:Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO.intsGppMask;
}

/*******************************************************************************
* mvBoardMppGet - Get board dependent MPP register value
*
* DESCRIPTION:
*		MPP settings are derived from board design.
*		MPP group consist of 8 MPPs. An MPP group represent MPP 
*		control register.
*       This function retrieves board dependend MPP register value.
*
* INPUT:
*       mppGroupNum - MPP group number. 
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit value describing MPP control register value. 
*
*******************************************************************************/
MV_U32 mvBoardMppGet(MV_U32 mppGroupNum)
{
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardMppGet:Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO.mppGroup[mppGroupNum];
}


/* Board devices API managments */

/*******************************************************************************
* mvBoardGetDeviceNumber - Get number of device of some type on the board
*
* DESCRIPTION:
*
* INPUT:
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		number of those devices else the function returns 0
*
*
*******************************************************************************/
MV_32 mvBoardGetDevicesNumber(MV_BOARD_DEV_TYPE devType)
{
	MV_U32	foundIndex=0,devNum;
	MV_U32 boardId= mvBoardIdGet();
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("mvBoardGetDeviceNumber:Board unknown.\n");
		return 0xFFFFFFFF;

	}

	for (devNum = START_DEV_CS; devNum < MV_DEV_MAX_CS; devNum++)
	{
		if (BOARD_INFO.devCsInfo[devNum].devType == devType)
		{
			foundIndex++;
		}
	}

    return foundIndex;

}

/*******************************************************************************
* mvBoardGetDeviceBaseAddr - Get base address of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		Base address else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceBaseAddr(MV_32 devIndex, MV_BOARD_DEV_TYPE devType)
{
	MV_32 devNum;

	devNum = boardGetDevCSNum(devIndex,devType);

	if (devNum != 0xFFFFFFFF)
	{
		return mvCpuIfTargetWinBaseLowGet(DEV_TO_TARGET(devNum));
		
	}

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardGetDeviceBusWidth - Get Bus width of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		Bus width else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceBusWidth(MV_32 devIndex, MV_BOARD_DEV_TYPE devType)
{
	MV_32 devNum;

	devNum = boardGetDevCSNum(devIndex,devType);

	if (devNum != 0xFFFFFFFF)
	{
		return mvDevWidthGet(devNum);
		
	}

	return 0xFFFFFFFF;

}

/*******************************************************************************
* mvBoardGetDeviceWidth - Get dev width of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		dev width else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceWidth(MV_32 devIndex, MV_BOARD_DEV_TYPE devType)
{
	MV_32 devNum;
	MV_U32 boardId= mvBoardIdGet();
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("Board unknown.\n");
		return 0xFFFFFFFF;

	}

	devNum = boardGetDevCSNum(devIndex,devType);

	if (devNum != 0xFFFFFFFF)
	{
		return BOARD_INFO.devCsInfo[devNum].devWidth;
		
	}

	return 0xFFFFFFFF;

}

/*******************************************************************************
* boardGetDevCSNum - returns the Device CS number of a device on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		dev number else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_U32 boardGetDevCSNum(MV_32 devIndex, MV_BOARD_DEV_TYPE devType)
{
	MV_U32	foundIndex=0,devNum;
	MV_U32 boardId= mvBoardIdGet();
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	if (boardId > MV_MAX_BOARD_ID)
	{
		mvOsPrintf("boardGetDevCSNum: Board unknown.\n");
		return 0xFFFFFFFF;

	}

	/* because some restrictions like in U-boot that always expect the BootFlash to be
	the first flash - we want always the Boot CS to be the first device of its kind ,
	so we always will start searching from there and then search the other */

	if (BOARD_INFO.devCsInfo[BOOT_CS].devType == devType)
	{
		if (foundIndex == devIndex) return BOOT_CS;
		foundIndex++;
	}

	for (devNum = START_DEV_CS; devNum < MV_DEV_MAX_CS; devNum++)
	{

		if (devNum == BOOT_CS) continue;

		if (BOARD_INFO.devCsInfo[devNum].devType == devType)
		{
            if (foundIndex == devIndex)
			{
				return devNum;
			}
			foundIndex++;
		}
	}

	/* device not found */
	return 0xFFFFFFFF;


}

/*******************************************************************************
* mvBoardRtcTwsiAddrTypeGet - 
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
*
*******************************************************************************/
MV_U8 mvBoardRtcTwsiAddrTypeGet()
{
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();
	return BOARD_INFO.rtcTwsiAddrType;
}

/*******************************************************************************
* mvBoardRtcTwsiAddrGet - 
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
*
*******************************************************************************/
MV_U8 mvBoardRtcTwsiAddrGet()
{
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();
	return BOARD_INFO.rtcTwsiAddr;
}

/*******************************************************************************
* mvBoardFirstPciSlotDevNumGet - 
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
*
*******************************************************************************/
MV_U32 mvBoardFirstPciSlotDevNumGet(MV_U32 pciIf)
{
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();
	return BOARD_INFO.pciBoardIf[pciIf].firstSlotDevNum;
}

/*******************************************************************************
* mvBoardPciSlotsNumGet - 
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
*
*******************************************************************************/
MV_U32 	    mvBoardPciSlotsNumGet(MV_U32 pciIf)
{
	MV_U32 boardId;
	MV_BOARD_INFO	*boardInfoTbl=NULL;

	boardInfoTbl = MV_CURRENT_TABLE_INFO;

	boardId = mvBoardIdGet();
	return BOARD_INFO.pciBoardIf[pciIf].pciSlotsNum;
}

/*******************************************************************************
* mvBoardSlicGpioPinGet - 
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
*
*******************************************************************************/
MV_U32 mvBoardSlicGpioPinGet(MV_U32 slicNum)
{
	MV_U32 boardId;
	boardId = mvBoardIdGet();

	switch (boardId)
	{
	case DB_88F5181L_DDR2_2XTDM:
		if (0 == slicNum)
		{
			return 8;
		}
		else if (1 == slicNum)
		{
			return 9;
		}
		else return -1;
		break;
	case RD_88F5181L_VOIP_FE:
		if (0 == slicNum)
		{
			return 2;
		}
		else if (1 == slicNum)
		{
			return 5;
		}
		else return -1;
		break;
	default:
		return -1;
		break;

	}
}

