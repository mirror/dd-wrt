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
#include "cpu/mvCpu.h"
#include "cntmr/mvCntmr.h"
#include "gpp/mvGpp.h"
#include "twsi/mvTwsi.h"
#include "pci-if/mvPciIf.h"
#include "device/mvDevice.h"

#if defined(DB_CUSTOMER)
#include "boardEnv/mvCustomerBoardEnv.c"
#elif defined(MV_88F5180N)
#include "boardEnv/mv88F5180NBoardEnv.c"
#elif defined(MV_88F5181)
#include "boardEnv/mv88F5181BoardEnv.c"
#elif defined(MV_88F5181L)
#include "boardEnv/mv88F5181LBoardEnv.c"
#elif defined(MV_88F5182)
#include "boardEnv/mv88F5182BoardEnv.c"
#elif defined(MV_88F5082)
#include "boardEnv/mv88F5082BoardEnv.c"
#elif defined(MV_88W8660)
#include "boardEnv/mv88W8660BoardEnv.c"
#elif defined(MV_88F1281)
#include "boardEnv/mv88F1281BoardEnv.c"
#elif defined(MV_88F6082)
#include "boardEnv/mv88F6082BoardEnv.c"
#elif defined(MV_88F6183)
#include "boardEnv/mv88F6183BoardEnv.c"
#endif

/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif

extern MV_CPU_ARM_CLK _cpuARMDDRCLK[];

#define CODE_IN_ROM				MV_FALSE
#define CODE_IN_RAM				MV_TRUE

#define BOARD_INFO(boardId)	boardInfoTbl[boardId - BOARD_ID_BASE]

/* Locals */
static MV_DEV_CS_INFO*  boardGetDevEntry(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);

MV_U32 tClkRate   = -1;


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
	MV_U32 csNum;
	MV_U32 devBankParam=0;
	MV_U32 boardId= mvBoardIdGet();

#if defined(MV_INCLUDE_GIG_ETH)
	MV_U32	regVal;
#endif



	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardEnvInit:Board unknown.\n");
		return;

	}

	/* Set GPP Out value */
	MV_REG_WRITE(GPP_DATA_OUT_REG(0), BOARD_INFO(boardId)->gppOutVal);

	/* set GPP polarity */
	mvGppPolaritySet(0, 0xFFFFFFFF, BOARD_INFO(boardId)->gppPolarityVal);

	/* Set GPP Out Enable*/
	mvGppTypeSet(0, 0xFFFFFFFF, BOARD_INFO(boardId)->gppOutEnVal);


	for (devNum = START_DEV_CS; devNum < BOARD_INFO(boardId)->numBoardDeviceIf; devNum++)
	{
		devBankParam = BOARD_INFO(boardId)->pDevCsInfo[devNum].params;
		csNum = BOARD_INFO(boardId)->pDevCsInfo[devNum].deviceCS;

		if (devBankParam == N_A) continue;

		if (devNum != MV_BOOTDEVICE_INDEX)
		{
			MV_REG_WRITE(DEV_BANK_PARAM_REG(csNum), devBankParam);
		}
		else
		{
			MV_U32 bootDevBankParam;

			/* for BootCS Only device width should be as in sample at
			reset */
			bootDevBankParam = MV_REG_READ(DEV_BANK_PARAM_REG(devNum));
			bootDevBankParam &= DBP_DEVWIDTH_MASK;
			devBankParam &= ~DBP_DEVWIDTH_MASK;
			devBankParam |= bootDevBankParam;

			MV_REG_WRITE(DEV_BANK_PARAM_REG(csNum) , devBankParam);
		}
	}

#if defined(MV_INCLUDE_NAND)

	if (mvCtrlNandSupport())
	{
	    /* If we are booting from NAND MPPs should be modified */
	    /* Check NAND connected to boot device */
#if defined(MV_88F6082)
	     if (((MV_REG_READ(MPP_SAMPLE_AT_RESET) & MSAR_DBOOT_MODE_MASK) == MSAR_DBOOT_NAND) ||
	     ((MV_REG_READ(MPP_SAMPLE_AT_RESET) & MSAR_DBOOT_MODE_MASK) == MSAR_IDBOOT_NAND))
	    {
	    }
#else
	    if (MV_REG_READ(DEV_NAND_CTRL_REG) & 0x1)
	    {
	    }
#endif
	    else
	    {

		for (devNum = START_DEV_CS; devNum < BOARD_INFO(boardId)->numBoardDeviceIf; devNum++)
		{
			if (boardGetDevEntry(devNum, BOARD_DEV_NAND_FLASH) != NULL)
			{
				/* We always use don't care mode. */
				mvDevNandSet((boardGetDevEntry(devNum, BOARD_DEV_NAND_FLASH))->deviceCS, 0);
			}
		}
	    }

	}
#endif /* MV_INCLUDE_NAND */

#if defined(MV_INCLUDE_GIG_ETH)

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

#endif /* MV_INCLUDE_GIG_ETH */

}

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

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsSPrintf (pNameBuff, "Board unknown.\n");
		return MV_ERROR;

	}

	mvOsSPrintf (pNameBuff, "%s",BOARD_INFO(boardId)->boardName);

	return MV_OK;
}

/*******************************************************************************
* mvBoardIsPortInSgmii -
*
* DESCRIPTION:
*       This routine returns MV_TRUE for port number works in SGMII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in SGMII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInSgmii(MV_U32 ethPortNum)
{
    MV_BOOL ethPortSgmiiSupport[BOARD_ETH_PORT_NUM] = MV_ETH_PORT_SGMII;

    if(ethPortNum >= BOARD_ETH_PORT_NUM)
    {
	    mvOsPrintf ("Invalid portNo=%d\n", ethPortNum);
		return MV_FALSE;
	}
    return ethPortSgmiiSupport[ethPortNum];
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
*       32bit describing Phy address, -1 if the port number is wrong.
*
*******************************************************************************/
MV_32 mvBoardPhyAddrGet(MV_U32 ethPortNum)
{
	MV_U32 boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardPhyAddrGet: Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO(boardId)->pBoardMacInfo[ethPortNum].boardEthSmiAddr;
}

/*******************************************************************************
* mvBoardMacSpeedGet - Get the Mac speed
*
* DESCRIPTION:
*       This routine returns the Mac speed if pre define of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BOARD_MAC_SPEED, -1 if the port number is wrong.
*
*******************************************************************************/
MV_BOARD_MAC_SPEED      mvBoardMacSpeedGet(MV_U32 ethPortNum)
{
	MV_U32 boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardMacSpeedGet: Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO(boardId)->pBoardMacInfo[ethPortNum].boardMacSpeed;
}

/*******************************************************************************
* mvBoardLinkStatusIrqGet - Get the IRQ number for the link status indication
*
* DESCRIPTION:
*       This routine returns the IRQ number for the link status indication.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       the number of the IRQ for the link status indication, -1 if the port 
*	number is wrong or if not relevant.
*
*******************************************************************************/
MV_32	mvBoardLinkStatusIrqGet(MV_U32 ethPortNum)
{
	MV_U32 boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardLinkStatusIrqGet: Board unknown.\n");
		return MV_ERROR;
	}

	return BOARD_INFO(boardId)->pSwitchInfo[ethPortNum].linkStatusIrq;
}

/*******************************************************************************
* mvBoardSwitchPortGet - Get the mapping between the board connector and the 
* Ethernet Switch port
*
* DESCRIPTION:
*       This routine returns the matching Switch port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*	boardPortNum - logical number of the connector on the board
*
* OUTPUT:
*       None.
*
* RETURN:
*       the matching Switch port, -1 if the port number is wrong or if not relevant.
*
*******************************************************************************/
MV_32	mvBoardSwitchPortGet(MV_U32 ethPortNum, MV_U8 boardPortNum)
{
	MV_U32 boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardSwitchPortGet: Board unknown.\n");
		return MV_ERROR;
	}
	if (boardPortNum >= BOARD_ETH_SWITCH_PORT_NUM)
	{
		mvOsPrintf("mvBoardSwitchPortGet: Illegal board port number.\n");
		return MV_ERROR;
	}

	return BOARD_INFO(boardId)->pSwitchInfo[ethPortNum].qdPort[boardPortNum];
}

/*******************************************************************************
* mvBoardSwitchCpuPortGet - Get the the Ethernet Switch CPU port
*
* DESCRIPTION:
*       This routine returns the Switch CPU port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       the Switch CPU port, -1 if the port number is wrong or if not relevant.
*
*******************************************************************************/
MV_32	mvBoardSwitchCpuPortGet(MV_U32 ethPortNum)
{
	MV_U32 boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardSwitchCpuPortGet: Board unknown.\n");
		return MV_ERROR;
	}

	return BOARD_INFO(boardId)->pSwitchInfo[ethPortNum].qdCpuPort;
}

/*******************************************************************************
* mvBoardSmiScanModeGet - Get Switch SMI scan mode
*
* DESCRIPTION:
*       This routine returns Switch SMI scan mode.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       1 for SMI_MANUAL_MODE, -1 if the port number is wrong or if not relevant.
*
*******************************************************************************/
MV_32	mvBoardSmiScanModeGet(MV_U32 ethPortNum)
{
	MV_U32 boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardSmiScanModeGet: Board unknown.\n");
		return MV_ERROR;
	}

	return BOARD_INFO(boardId)->pSwitchInfo[ethPortNum].smiScanMode;
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

    if (-1 == tClkRate)
    {
	/* Auto calc Tclk using counters */
#ifdef MV_TCLK_CALC
    	MV_U32 ticks;
	MV_U32 refClkDevBitRate;
	MV_U32 refClkDevBit;
	MV_U32 countNum= MV_TCLK_CNTR;		/* Counter 1 is used for Tclk measurment */

	refClkDevBit = MV_REF_CLK_DEV_BIT;
	refClkDevBitRate = MV_REF_CLK_BIT_RATE;

	/* Stop counter activity */
	mvCntmrDisable(countNum);

	/* load value onto counter\timer */
	mvCntmrLoad(countNum,0xffffffff);

	/* set the counter to load in the first time */
	mvCntmrWrite(countNum,0xffffffff);

	/* Set input indication pin as input */
	MV_REG_BIT_SET(GPP_DATA_OUT_EN_REG(0),(1 << MV_REF_CLK_INPUT_GPP));

	/* Enable interrupt as edge */
	MV_REG_BIT_SET(GPP_INT_MASK_REG(0),(1 << MV_REF_CLK_INPUT_GPP) );

	/* This function is blocking. It returns only when reference clock is rising */
	/* Clear Interrupt cause */
	MV_REG_WRITE(GPP_INT_CAUSE_REG(0),0);

	do
	{
		if ( MV_REG_READ(GPP_INT_CAUSE_REG(0)) & (1 << MV_REF_CLK_INPUT_GPP)) break;
	}while (1);


	/* set control for timer \ cunter and enable */
	mvCntmrEnable(countNum);

	/* This function is blocking. It returns only when reference clock is rising */
	/* Clear Interrupt cause */
	MV_REG_WRITE(GPP_INT_CAUSE_REG(0),0);

	do
	{
	    do
	    {
		if ( MV_REG_READ(GPP_INT_CAUSE_REG(0)) & (1 << MV_REF_CLK_INPUT_GPP)) break;
	    }while (1);
	    MV_REG_WRITE(GPP_INT_CAUSE_REG(0),0);
	}while (refClkDevBit--);

	/* Timer counts back. We need total num of ticks from count begin	*/
	ticks = ~MV_REG_READ(CNTMR_VAL_REG(countNum));

	/* Disable interrupt */
	MV_REG_BIT_RESET(GPP_INT_MASK_REG(0),(1 << MV_REF_CLK_INPUT_GPP) );

	/* Clear Interrupt cause */
	MV_REG_BIT_RESET(GPP_INT_CAUSE_REG(0),(1 << MV_REF_CLK_INPUT_GPP));

	/* Release the reference clock device and stop timer 				*/
	mvCntmrDisable(countNum);

	tmpTClkRate = (ticks/MV_REF_CLK_DEV_BIT) * refClkDevBitRate;

#elif defined(DB_FPGA)
	tmpTClkRate = MV_DB_FPGA_TCLK;
#elif defined(TCLK_AUTO_DETECT)

	tmpTClkRate = MV_REG_READ(MPP_SAMPLE_AT_RESET);

	tmpTClkRate &= MSAR_TCLCK_MASK;

	switch (tmpTClkRate)
	{
	case MSAR_TCLCK_133:
		tmpTClkRate = MV_BOARD_TCLK_133MHZ;
		break;
#if !defined(MV_88F6082) && !defined(MV_88F6183)
	case MSAR_TCLCK_150:
		tmpTClkRate = MV_BOARD_TCLK_150MHZ;
		break;
#endif
	case MSAR_TCLCK_166:
		tmpTClkRate = MV_BOARD_TCLK_166MHZ;
		break;
	}

#else

	tmpTClkRate = MV_BOARD_DEFAULT_TCLK;

#endif

	tClkRate = tmpTClkRate;
    }
    else
	tmpTClkRate = tClkRate;

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
#ifndef DB_FPGA
	MV_U32	tmp;
#endif

#ifdef DB_FPGA
	tmpSysClkRate = MV_DB_FPGA_CPU_CLK;
#else
#ifdef SYSCLK_AUTO_DETECT
		tmp = MV_REG_READ(MPP_SAMPLE_AT_RESET);
		tmpSysClkRate = tmp & MSAR_ARMDDRCLCK_MASK;
		tmpSysClkRate = tmpSysClkRate >> MSAR_ARMDDRCLCK_OFFS;
		if ((mvCtrlModelGet() == MV_5281_DEV_ID) || (mvCtrlModelGet() == MV_1281_DEV_ID))
			if(tmp & MSAR_ARMDDRCLCK_H_MASK)
				tmpSysClkRate |= BIT4;

		tmpSysClkRate = _cpuARMDDRCLK[tmpSysClkRate].ddrClk;

#else
		tmpSysClkRate = MV_BOARD_DEFAULT_SYSCLK;
#endif
#endif /* DB_FPGA */
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
	MV_U32 	boardId, addr,val = 0,totalMask, currentBitMask = 1,i;

	boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardDebug7Seg:Board unknown.\n");
		return;

	}

	/* Check if 7Segments is wired to CS */
	addr = mvBoardGetDeviceBaseAddr(0, BOARD_DEV_SEVEN_SEG);

	if (addr != 0xFFFFFFFF)
	{
		hexNum = *(volatile MV_U32*)((MV_U32)CPU_MEMIO_UNCACHED_ADDR(addr) + 													(MV_U32)((hexNum & 0xf)  << 4));

		return;

	}
	else
	{

	    /* the 7seg is wired to GPPs */
	    totalMask = (1 << BOARD_INFO(boardId)->activeLedsNumber) -1;
	    hexNum &= totalMask;
	    totalMask = 0;

	    for (i = 0 ; i < BOARD_INFO(boardId)->activeLedsNumber ; i++)
	    {
		if (hexNum & currentBitMask)
		{
		    val |= (1 << BOARD_INFO(boardId)->pLedGppPin[i]);
		}

		totalMask |= (1 << BOARD_INFO(boardId)->pLedGppPin[i]);

		currentBitMask = (currentBitMask << 1);
	    }

	    if (BOARD_INFO(boardId)->ledsPolarity)
	    {
		mvGppValueSet(0, totalMask, val);
	    }
	    else
	    {
		mvGppValueSet(0, totalMask, ~val);
	    }
	}
}

#if defined(MV_INCLUDE_PCI)

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
MV_32 mvBoardPciGpioPinGet(MV_U32 pciIf, MV_U32 devNum, MV_U32 intPin)
{
	int i;
	MV_U32 boardId;

	/* Convert PciIf to the real PCi Interface number */
	pciIf = mvPciRealIfNumGet(pciIf);

	boardId = mvBoardIdGet();


	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardPciGpioPinGet:Board ID %d unknown.\n", boardId);
		return MV_ERROR;

	}

	if ((DB_88F5181_DDR1_PRPMC  == boardId) ||
        	(DB_88F5181_DDR1_PEXPCI == boardId) ||
        	(DB_88F5181_DDR1_MNG == boardId))
	{
        /* These boards are NOT backplans. PCI interrupt connectivity   */
        /* information of a specifc user backplain, which to install    */
        /* those boards, is unknown.                                    */
        /* Marvell general HAL provides default PCI definition for      */
        /* these add-in cards. Each user should modify this             */
        /* configuration according to the backplain in use.             */
        	for (i = 0; i < BOARD_INFO(boardId)->numBoardPciIf; i++)
			if (BOARD_INFO(boardId)->pBoardPciIf[i].pciDevNum == devNum)
        			return (MV_U32)BOARD_INFO(boardId)->pBoardPciIf[i].pciGppIntMap[intPin - 1];
    	}

    	if (BOARD_INFO(boardId)->pBoardPciIf[pciIf].pciDevNum == (MV_U8)N_A)
	{
		mvOsPrintf("mvBoardPciGpioPinGet: ERR. Could not find GPP pin " \
                   "assignment for pciIf %d devNum %d intPin %d\n",
                   pciIf, devNum, intPin);
		return N_A;
	}


        for (i = 0; i < BOARD_INFO(boardId)->numBoardPciIf; i++)
		if (BOARD_INFO(boardId)->pBoardPciIf[i].pciDevNum == devNum)
        		return (MV_U32)BOARD_INFO(boardId)->pBoardPciIf[i].pciGppIntMap[intPin - 1];

	mvOsPrintf("mvBoardPciGpioPinGet:Illigal device number %d\n", devNum);
	return N_A;
}
#endif

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
MV_32 mvBoardRTCGpioPinGet(MV_VOID)
{
	MV_U32 boardId, i;

	boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardRTCGpioPinGet:Board unknown.\n");
		return MV_ERROR;

	}

        for (i = 0; i < BOARD_INFO(boardId)->numBoardGppInfo; i++)
		if (BOARD_INFO(boardId)->pBoardGppInfo[i].devClass == BOARD_DEV_RTC)
        		return (MV_U32)BOARD_INFO(boardId)->pBoardGppInfo[i].gppPinNum;

	return MV_ERROR;
}

/*******************************************************************************
* mvBoarGpioPinGet - mvBoarGpioPinGet
*
* DESCRIPTION:
*
* INPUT:
*		class - MV_BOARD_DEV_CLASS enum.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoarGpioPinNumGet(MV_BOARD_DEV_CLASS class)
{
	MV_U32 boardId, i;

	boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardRTCGpioPinGet:Board unknown.\n");
		return MV_ERROR;

	}

        for (i = 0; i < BOARD_INFO(boardId)->numBoardGppInfo; i++)
		if (BOARD_INFO(boardId)->pBoardGppInfo[i].devClass == class)
        		return (MV_U32)BOARD_INFO(boardId)->pBoardGppInfo[i].gppPinNum;

	return MV_ERROR;
}


/*******************************************************************************
* mvBoardReset - mvBoardReset
*
* DESCRIPTION:
*			Reset the board
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       None
*
*******************************************************************************/
#define WAIT_DELAY \
	{ volatile int dummy; int i; for (i=0; i<500; i++) dummy = i; }

MV_VOID	mvBoardReset(MV_VOID)
{
	MV_32 resetPin,boardId = mvBoardIdGet();
	
	/* Get gpp reset pin if define */
	resetPin = mvBoardResetGpioPinGet();
	if (resetPin != MV_ERROR)
	{
		if ((boardId == RD_88F6082_NAS)
			|| (boardId == RD_88F6082_MICRO_DAS_NAS)
			|| (boardId == RD_88F6082_DX243_24G))
		{
			/* Set DRAM into self refresh mode */
			MV_REG_WRITE(SDRAM_CONFIG_REG, 0xfa14410);
			MV_REG_WRITE(SDRAM_OPERATION_REG, 0x7);
			
			/* wait 500 clock cycles */
			WAIT_DELAY

			MV_REG_BIT_SET( GPP_DATA_OUT_REG(0) ,(1 << resetPin));
			MV_REG_BIT_RESET( GPP_DATA_OUT_EN_REG(0) ,(1 << resetPin));
		}

        	MV_REG_BIT_RESET( GPP_DATA_OUT_REG(0) ,(1 << resetPin));
		MV_REG_BIT_RESET( GPP_DATA_OUT_EN_REG(0) ,(1 << resetPin));

	}
	else
	{
	    /* No gpp reset pin was found, try to reset ussing
	    system reset out */
	    MV_REG_BIT_SET( CPU_RSTOUTN_MASK_REG , BIT2);
	    MV_REG_BIT_SET( CPU_SYS_SOFT_RST_REG , BIT0);
	}
	
	while(1);
}

/*******************************************************************************
* mvBoardResetGpioPinGet - mvBoardResetGpioPinGet
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
MV_32 mvBoardResetGpioPinGet(MV_VOID)
{
	MV_U32 boardId, i;

	boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardRTCGpioPinGet:Board unknown.\n");
		return MV_ERROR;

	}

        for (i = 0; i < BOARD_INFO(boardId)->numBoardGppInfo; i++)
		if (BOARD_INFO(boardId)->pBoardGppInfo[i].devClass == BOARD_DEV_RESET)
        		return (MV_U32)BOARD_INFO(boardId)->pBoardGppInfo[i].gppPinNum;

	return MV_ERROR;
}
#if defined(MV_INCLUDE_SDIO)
/*******************************************************************************
* mvBoardSDIOGpioPinGet - mvBoardSDIOGpioPinGet
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
MV_32  mvBoardSDIOGpioPinGet(MV_VOID)
{
	MV_U32 boardId, i;

	boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardRTCGpioPinGet:Board unknown.\n");
		return MV_ERROR;

	}

        for (i = 0; i < BOARD_INFO(boardId)->numBoardGppInfo; i++)
		if (BOARD_INFO(boardId)->pBoardGppInfo[i].devClass == BOARD_DEV_SDIO_DETECT)
        		return (MV_U32)BOARD_INFO(boardId)->pBoardGppInfo[i].gppPinNum;

	return MV_ERROR;
}
#endif

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
MV_32 mvBoardUSBVbusGpioPinGet(int devId)
{
	MV_U32 boardId, i, indexFound = 0;

	boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardUSBVbusGpioPinGet:Board unknown.\n");
		return MV_ERROR;

	}

        for (i = 0; i < BOARD_INFO(boardId)->numBoardGppInfo; i++)
		if (BOARD_INFO(boardId)->pBoardGppInfo[i].devClass == BOARD_DEV_USB_VBUS)
		{
			if (indexFound == devId)
        			return (MV_U32)BOARD_INFO(boardId)->pBoardGppInfo[i].gppPinNum;
			else
				indexFound++;
		}

	return MV_ERROR;
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
MV_32 mvBoardGpioIntMaskGet(MV_VOID)
{
	MV_U32 boardId;

	boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardGpioIntMaskGet:Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO(boardId)->intsGppMask;
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
MV_32 mvBoardMppGet(MV_U32 mppGroupNum)
{
	MV_U32 boardId;

	boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardMppGet:Board unknown.\n");
		return MV_ERROR;

	}

	return BOARD_INFO(boardId)->pBoardMppConfigValue[0].mppGroup[mppGroupNum];
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
MV_32 mvBoardGetDevicesNumber(MV_BOARD_DEV_CLASS devClass)
{
	MV_U32	foundIndex=0,devNum;
	MV_U32 boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardGetDeviceNumber:Board unknown.\n");
		return 0xFFFFFFFF;

	}

	for (devNum = START_DEV_CS; devNum < BOARD_INFO(boardId)->numBoardDeviceIf; devNum++)
	{
		if (BOARD_INFO(boardId)->pDevCsInfo[devNum].devClass == devClass)
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
MV_32 mvBoardGetDeviceBaseAddr(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry;

	devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
	{
		return mvCpuIfTargetWinBaseLowGet(DEV_TO_TARGET(devEntry->deviceCS));

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
MV_32 mvBoardGetDeviceBusWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry;

	devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
	{
		return mvDevWidthGet(devEntry->deviceCS);

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
MV_32 mvBoardGetDeviceWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry;
	MV_U32 boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("Board unknown.\n");
		return 0xFFFFFFFF;

	}


	devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
		return devEntry->devWidth;

	return MV_ERROR;

}

/*******************************************************************************
* mvBoardGetDeviceWinSize - Get the window size of a device existing on the board
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
*		window size else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceWinSize(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry;
	MV_U32 boardId = mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("Board unknown.\n");
		return 0xFFFFFFFF;
	}

	devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
	{
		return mvCpuIfTargetWinSizeGet(DEV_TO_TARGET(devEntry->deviceCS));
	}

	return 0xFFFFFFFF;
}


/*******************************************************************************
* boardGetDevEntry - returns the entry pointer of a device on the board
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
*		dev number else the function returns 0x0
*
*
*******************************************************************************/
static MV_DEV_CS_INFO*  boardGetDevEntry(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_U32	foundIndex=0,devIndex;
	MV_U32 boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("boardGetDevEntry: Board unknown.\n");
		return NULL;

	}

	/* because some restrictions like in U-boot that always expect the BootFlash to be
	the first flash - we want always the Boot CS to be the first device of its kind ,
	so we always will start searching from there and then search the other */

	for (devIndex = START_DEV_CS; devIndex < BOARD_INFO(boardId)->numBoardDeviceIf; devIndex++)
	{
	    if ((BOARD_INFO(boardId)->pDevCsInfo[devIndex].devClass == devClass) &&
			(BOARD_INFO(boardId)->pDevCsInfo[devIndex].deviceCS == MV_BOOTDEVICE_INDEX))
	    {
			if (foundIndex == devNum)
				return &(BOARD_INFO(boardId)->pDevCsInfo[devIndex]);
			else
				foundIndex++;
	    }
	}


	for (devIndex = START_DEV_CS; devIndex < BOARD_INFO(boardId)->numBoardDeviceIf; devIndex++)
	{

		if (BOARD_INFO(boardId)->pDevCsInfo[devIndex].deviceCS == MV_BOOTDEVICE_INDEX)
		     continue;

		if (BOARD_INFO(boardId)->pDevCsInfo[devIndex].devClass == devClass)
		{
            		if (foundIndex == devNum)
			{
				return &(BOARD_INFO(boardId)->pDevCsInfo[devIndex]);
			}
			foundIndex++;
		}
	}

	/* device not found */
	return NULL;
}


MV_U32 boardGetDevCSNum(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry;
	MV_U32 boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("Board unknown.\n");
		return 0xFFFFFFFF;

	}


	devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
		return devEntry->deviceCS;

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
	int i;
	MV_U32 boardId= mvBoardIdGet();

	for (i = 0; i < BOARD_INFO(boardId)->numBoardTwsiDev; i++)
		if (BOARD_INFO(boardId)->pBoardTwsiDev[i].devClass == BOARD_DEV_RTC)
			return BOARD_INFO(boardId)->pBoardTwsiDev[i].twsiDevAddrType;
	return (MV_ERROR);
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
	int i;
	MV_U32 boardId= mvBoardIdGet();

	for (i = 0; i < BOARD_INFO(boardId)->numBoardTwsiDev; i++)
		if (BOARD_INFO(boardId)->pBoardTwsiDev[i].devClass == BOARD_DEV_RTC)
			return BOARD_INFO(boardId)->pBoardTwsiDev[i].twsiDevAddr;
	return (0xFF);
}

/*******************************************************************************
* mvBoardA2DTwsiAddrTypeGet -
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
MV_U8 mvBoardA2DTwsiAddrTypeGet()
{
	int i;
	MV_U32 boardId= mvBoardIdGet();

	for (i = 0; i < BOARD_INFO(boardId)->numBoardTwsiDev; i++)
		if (BOARD_INFO(boardId)->pBoardTwsiDev[i].devClass == BOARD_DEV_AUDIO_DEC)
			return BOARD_INFO(boardId)->pBoardTwsiDev[i].twsiDevAddrType;
	return (MV_ERROR);
}

/*******************************************************************************
* mvBoardA2DTwsiAddrGet -
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
MV_U8 mvBoardA2DTwsiAddrGet()
{
	int i;
	MV_U32 boardId= mvBoardIdGet();

	for (i = 0; i < BOARD_INFO(boardId)->numBoardTwsiDev; i++)
		if (BOARD_INFO(boardId)->pBoardTwsiDev[i].devClass == BOARD_DEV_AUDIO_DEC)
			return BOARD_INFO(boardId)->pBoardTwsiDev[i].twsiDevAddr;
	return (0xFF);
}


#if defined(MV_INCLUDE_PCI)

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
MV_32 mvBoardFirstPciSlotDevNumGet(MV_U32 pciIf)
{
	MV_U32 boardId;

	boardId = mvBoardIdGet();
	return BOARD_INFO(boardId)->pBoardPciIf[pciIf].pciDevNum;
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
MV_32 	    mvBoardPciSlotsNumGet(MV_U32 pciIf)
{
	MV_U32 boardId;

	boardId = mvBoardIdGet();
	return BOARD_INFO(boardId)->numBoardPciIf;
}

/*******************************************************************************
* mvBoardIsOurPciSlot -  Return true for lot number which is defined under
*			 the bus number.
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
MV_BOOL		mvBoardIsOurPciSlot(MV_U32 busNum, MV_U32 slotNum)
{
	MV_U32 boardId= mvBoardIdGet();
	MV_U32 localBusNum= mvPciLocalBusNumGet(PCI_DEFAULT_IF);
	int i;

	/* Our device number */
	if (slotNum == mvPciLocalDevNumGet(PCI_DEFAULT_IF))
	{
		return MV_TRUE;
	}

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardIsOurPciSlot: Board unknown.\n");
		return MV_FALSE;
	}

	if (localBusNum != busNum)
	{
		mvOsPrintf("mvBoardIsOurPciSlot: localBusNum %x != busNum %x.\n", localBusNum, busNum);
		return MV_FALSE;
	}

	for (i = 0; i < BOARD_INFO(boardId)->numBoardPciIf; i++)
		if (BOARD_INFO(boardId)->pBoardPciIf[i].pciDevNum == slotNum)
			return MV_TRUE;

	return MV_FALSE;
}
/*******************************************************************************
* mvBoardPciIsMonarch -
*
* DESCRIPTION:
*       This function is target to PRPMC type boards. In those boards there is

*       Check according to PCI arbiter control register if run with monarch
*
* INPUT:
*
* OUTPUT:
*
* RETURN:
*       MV_TRUE if the PRPMC is monarch, MV_FALSE otherwise.
*
*******************************************************************************/
MV_BOOL mvBoardPciIsMonarch(void)
{
#if defined(DB_MNG)
    /* the u-boot set the arbitter for DB_88F5181_DDR1_MNG boards */
	if((MV_REG_READ(PCI_ARBITER_CTRL_REG(0)) & PACR_ARB_ENABLE) == 0)
        return MV_TRUE;
#endif

    return MV_FALSE;
}
#endif /* #if defined(MV_INCLUDE_PCI) */
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
MV_32 mvBoardSlicGpioPinGet(MV_U32 slicNum)
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
		else return MV_ERROR;
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
		else return MV_ERROR;
		break;
	default:
		return MV_ERROR;
		break;

	}
}

/*******************************************************************************
* mvBoardStatusLedPinNumGet -
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN: Return the last debug led in the board info
*
*
*******************************************************************************/
MV_U8	    mvBoardStatusLedPinNumGet(MV_VOID)
{
	MV_U32 boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardIsOurPciSlot: Board unknown.\n");
		return MV_FALSE;
	}

	return BOARD_INFO(boardId)->pLedGppPin[BOARD_INFO(boardId)->activeLedsNumber-1];
}
/*******************************************************************************
* mvBoardSpecInitGet -
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN: Return MV_TRUE and parameters in case board need spesific phy init, 
* 	  otherwise return MV_FALSE. 
*
*
*******************************************************************************/

MV_BOOL mvBoardSpecInitGet(MV_U32* regOff, MV_U32* data)
{
	switch(mvBoardIdGet())
	{
		case RD_88F5181_POS_NAS:
			*regOff = 0x18;
			*data = 0x4151;
		    return MV_TRUE;

		default:
		    return MV_FALSE;
	}
}
/*******************************************************************************
* mvBoardLedNumGet -
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN: Return the last debug led in the board info
*
*
*******************************************************************************/

MV_U8	    mvBoardLedNumGet(MV_VOID)
{
	MV_U32 boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardIsOurPciSlot: Board unknown.\n");
		return MV_FALSE;
	}

	return BOARD_INFO(boardId)->activeLedsNumber;
}


/*******************************************************************************
* mvBoardStatusLed -
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN: MV_STATUS
*
*
*******************************************************************************/
MV_STATUS	    mvBoardStatusLed(MV_BOOL status)
{
	int val;
	int mask;
	MV_U32 boardId;

	/* Check first that there are debug leds on the board */
	if (mvBoardLedNumGet() == 0)
		return MV_TRUE;

	boardId= mvBoardIdGet();

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))
	{
		mvOsPrintf("mvBoardIsOurPciSlot: Board unknown.\n");
		return MV_FALSE;
	}

	mask = val = (1 << mvBoardStatusLedPinNumGet());

	if (status)
		val = ~val;

	if (0 == BOARD_INFO(boardId)->ledsPolarity)
	{
		return mvGppValueSet(0, mask, val);
	}
	else
	{
		return mvGppValueSet(0, mask, ~val);
	}

}


/*******************************************************************************
* mvBoardNandWidthGet -
*
* DESCRIPTION: Get the width of the first NAND device in byte.
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN: 1, 2, 4 or MV_ERROR
*
*
*******************************************************************************/
/*  */
MV_32 mvBoardNandWidthGet(void)
{
	MV_U32 devNum;
	MV_U32 devWidth;
	MV_U32 boardId= mvBoardIdGet();

	for (devNum = START_DEV_CS; devNum < BOARD_INFO(boardId)->numBoardDeviceIf; devNum++)
	{
		devWidth = mvBoardGetDeviceWidth(devNum, BOARD_DEV_NAND_FLASH);
		if (devWidth != MV_ERROR)
			return (devWidth / 8);
	}
		
	/* NAND wasn't found */
	return MV_ERROR;
}
