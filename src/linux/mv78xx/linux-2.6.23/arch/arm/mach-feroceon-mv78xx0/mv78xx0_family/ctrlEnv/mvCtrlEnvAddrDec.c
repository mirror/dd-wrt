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
/*******************************************************************************
* mvCtrlEnvAddrDec.h - Marvell controller address decode library
*
* DESCRIPTION:
*       The Marvell controller device has a fully programmable address map. 
*       There is a separate address map to each of the interfaces:
*       - CPU address space
*       - PCI_0 address space
*       - PCI_1 address space
*       - Gigabit Ethernet/FIFO interface address space
*       - IDMA address space
*       - XOR DMA address space (where XOR available)
*       - Serial communication address space.
*       Each interface includes programmable address windows that allows it 
*       to access any of the MV6456x resources. Each window can map up to 
*       4 GB of address space.
*       This file contains Marvell Controller address decode library API 
*       for Gigabit Ethernet, Serial communication, IDMA and XOR that shares 
*       the same address space decode implementation.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

/* includes */
#include "ctrlEnv/mvCtrlEnvAddrDec.h"

/* #define MV_DEBUG */

/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	
							
/* Base Address Register	(BAR)											*/
#define BAR_TARGET_DRAM			DRAM_TARGET_ID
#define BAR_TARGET_DEVICE		DEV_TARGET_ID


#define BAR_TARGET_PCI0			PCI0_TARGET_ID
#define BAR_TARGET_PCI1			PCI0_TARGET_ID
#define BAR_TARGET_PCI2			PCI0_TARGET_ID
#define BAR_TARGET_PCI3			PCI1_TARGET_ID
#define BAR_TARGET_PCI4			PCI1_TARGET_ID
#define BAR_TARGET_PCI5			PCI1_TARGET_ID

/* DRAM attributes 	 */
#define BAR_DRAM_BANK_MASK		0xF
#define BAR_DRAM_BANK(bankNo)	(~(1 << (bankNo)) & BAR_DRAM_BANK_MASK)
#define BAR_DRAM_COHERENCY_OFFS	4			/* DRAM Cache coherency offset	*/
#define BAR_DRAM_COHERENCY_MASK	(0x3 << BAR_DRAM_COHERENCY_OFFS)
#define BAR_DRAM_NO_COHERENCY	(NO_COHERENCY << BAR_DRAM_COHERENCY_OFFS)
#define BAR_DRAM_WT_COHERENCY	(WT_COHERENCY << BAR_DRAM_COHERENCY_OFFS)
#define BAR_DRAM_WB_COHERENCY	(WB_COHERENCY << BAR_DRAM_COHERENCY_OFFS)
	
	/* Device attributes */
#define BAR_DEVICE_BANK_MASK	0x1F
#define BAR_DEVICE_BANK(bankNo)	(~(1 << (bankNo)) & BAR_DEVICE_BANK_MASK)
	
	/* PCI0/1 attributes */
#define BAR_PCI_BYTE_SWAP_OFFS	0
#define BAR_PCI_BYTE_SWAP_MASK	(0x3 << BAR_PCI_BYTE_SWAP_OFFS)
#define BAR_PCI_BYTE_SWAP		MV_BYTE_SWAP
#define	BAR_PCI_NO_SWAP			MV_NO_SWAP
#define	BAR_PCI_BYTE_WORD_SWAP	MV_BYTE_WORD_SWAP
#define	BAR_PCI_WORD_SWAP     	MV_WORD_SWAP
#define BAR_PCIX_NO_SNOOP_OFFS  2
#define BAR_PCIX_NO_SNOOP_MASK	(1 << BAR_PCIX_NO_SNOOP_OFFS)
#define BAR_PCI_BAR_TYPE_OFFS	3			/* PCI MEM/IO space				*/
#define BAR_PCI_BAR_TYPE_MASK	(1 << BAR_PCI_BAR_TYPE_OFFS)
#define BAR_PCI_IO_SPACE		(0 << BAR_PCI_BAR_TYPE_OFFS)
#define BAR_PCI_MEMORY_SPACE	(1 << BAR_PCI_BAR_TYPE_OFFS)
#define BAR_PCI_REQ64_MODE_OFFS	4			/* PCIx_REQ64n control			*/
#define BAR_PCI_REQ64_MODE_MASK	(1 << BAR_PCI_REQ64_MODE_OFFS)
#define BAR_PCI_REQ64_ALWAYS	(0 << BAR_PCI_REQ64_MODE_OFFS)
#define BAR_PCI_REQ64_PER_REQU	(1 << BAR_PCI_REQ64_MODE_OFFS)

#define BAR_PCI_LANE_SELECT_OFF 4
#define BAR_PCI_LANE_SELECT_MASK (0xf << BAR_PCI_LANE_SELECT_OFF)
#define BAR_PCI_LANE_0           (0xe << BAR_PCI_LANE_SELECT_OFF)
#define BAR_PCI_LANE_1           (0xd << BAR_PCI_LANE_SELECT_OFF)
#define BAR_PCI_LANE_2           (0xb << BAR_PCI_LANE_SELECT_OFF)
#define BAR_PCI_LANE_3           (0x7 << BAR_PCI_LANE_SELECT_OFF)

/* Size register */
#define BAR_SIZE_ALIGNMENT		0x10000	/* Minimum size 64K	*/

/* typedefs */

/* Locals */

MV_TARGET_ATTRIB	mvTargetDefaultsArray[] = 
{					
    	{0x0E,DRAM_TARGET_ID}, /* SDRAM_CS0 */		
    	{0x0D,DRAM_TARGET_ID}, /* SDRAM_CS1 */		
    	{0x0B,DRAM_TARGET_ID}, /* SDRAM_CS2 */		
    	{0x07,DRAM_TARGET_ID}, /* SDRAM_CS3 */		
    	{0x3E,DEV_TARGET_ID},  /* DEVICE_CS0 */		
    	{0x3D,DEV_TARGET_ID},  /* DEVICE_CS1 */		
    	{0x3B,DEV_TARGET_ID},  /* DEVICE_CS2 */		
    	{0x37,DEV_TARGET_ID},  /* DEVICE_CS3 */		
    	{0x2F,DEV_TARGET_ID},  /* DEV_BOOCS*/         
#if !defined(MV78XX0_Z0)
    	{0x1F,DEV_TARGET_ID},  /* DEV_SPI*/         
#endif
	{0xE0,PCI0_TARGET_ID}, /* PCI0_IO */		
	{0xE8,PCI0_TARGET_ID}, /* PCI0_MEM */		
	{0xD0,PCI0_TARGET_ID}, /* PCI1_IO */		
	{0xD8,PCI0_TARGET_ID}, /* PCI1_MEM */			
	{0xB0,PCI0_TARGET_ID}, /* PCI2_IO */		
	{0xB8,PCI0_TARGET_ID}, /* PCI2_MEM */		
	{0x70,PCI0_TARGET_ID}, /* PCI3_IO */		
	{0x78,PCI0_TARGET_ID}, /* PCI3_MEM */			
	{0xE0,PCI1_TARGET_ID}, /* PCI4_IO */		
	{0xE8,PCI1_TARGET_ID}, /* PCI4_MEM */		
	{0xD0,PCI1_TARGET_ID}, /* PCI5_IO */		
	{0xD8,PCI1_TARGET_ID}, /* PCI5_MEM */		
	{0xB0,PCI1_TARGET_ID}, /* PCI6_IO */		
	{0xB8,PCI1_TARGET_ID}, /* PCI6_MEM */		
	{0x70,PCI1_TARGET_ID}, /* PCI7_IO */		
	{0x78,PCI1_TARGET_ID}, /* PCI7_MEM */		
   	{0x01,CRYPT_TARGET_ID}, /* CRYPT_ENG */	
    	{0xFF,		0xFF} /* INTER_REGS */		
};

#define CTRL_DEC_BASE_OFFS		16
#define CTRL_DEC_BASE_MASK		(0xffff << CTRL_DEC_BASE_OFFS)
#define CTRL_DEC_BASE_ALIGNMENT	0x10000

#define CTRL_DEC_SIZE_OFFS		16
#define CTRL_DEC_SIZE_MASK		(0xffff << CTRL_DEC_SIZE_OFFS)
#define CTRL_DEC_SIZE_ALIGNMENT	0x10000

#define CTRL_DEC_WIN_EN			BIT0


/*******************************************************************************
* mvCtrlAddrDecToReg - Get address decode register format values
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*
* RETURN:
*
*******************************************************************************/
MV_STATUS mvCtrlAddrDecToReg(MV_ADDR_WIN *pAddrDecWin, MV_DEC_REGS *pAddrDecRegs)
{

	MV_U32 baseToReg=0 , sizeToReg=0;
    
	/* BaseLow[31:16] => base register [31:16]		*/
	baseToReg = pAddrDecWin->baseLow & CTRL_DEC_BASE_MASK;

	/* Write to address decode Base Address Register                  */
	pAddrDecRegs->baseReg &= ~CTRL_DEC_BASE_MASK;
	pAddrDecRegs->baseReg |= baseToReg;

	/* Get size register value according to window size						*/
	sizeToReg = ctrlSizeToReg(pAddrDecWin->size, CTRL_DEC_SIZE_ALIGNMENT);
	
	/* Size parameter validity check.                                   */
	if (-1 == sizeToReg)
	{
		return MV_BAD_PARAM;
	}

	/* set size */
	pAddrDecRegs->sizeReg &= ~CTRL_DEC_SIZE_MASK;
	pAddrDecRegs->sizeReg |= (sizeToReg << CTRL_DEC_SIZE_OFFS);
	

	return MV_OK;

}


/*******************************************************************************
* mvCtrlRegToAddrDec - Extract address decode struct from registers.
*
* DESCRIPTION:
*       This function extract address decode struct from address decode 
*       registers given as parameters.
*
* INPUT:
*       pAddrDecRegs - Address decode register struct.
*
* OUTPUT:
*       pAddrDecWin - Target window data structure.
*
* RETURN:
*		MV_BAD_PARAM if address decode registers data is invalid.
*
*******************************************************************************/
MV_STATUS mvCtrlRegToAddrDec(MV_DEC_REGS *pAddrDecRegs, MV_ADDR_WIN *pAddrDecWin)
{
	MV_U32 sizeRegVal;
	
	sizeRegVal = (pAddrDecRegs->sizeReg & CTRL_DEC_SIZE_MASK) >> 
					CTRL_DEC_SIZE_OFFS;

	pAddrDecWin->size = ctrlRegToSize(sizeRegVal, CTRL_DEC_SIZE_ALIGNMENT);


	/* Extract base address						*/
	/* Base register [31:16] ==> baseLow[31:16] 		*/
	pAddrDecWin->baseLow = pAddrDecRegs->baseReg & CTRL_DEC_BASE_MASK;

	pAddrDecWin->baseHigh =  0;

	return MV_OK;
    
}


/*******************************************************************************
* mvCtrlAddrDecToParams - Get address decode register format values
*
* DESCRIPTION:
*       This function returns the given address window information in the 
*       format of address decode base and size registers.
*
* INPUT:
*       pAddrDecWin - Target window data structure.
*
* OUTPUT:
*       pWinParam   - Address decode window parameters.
*
* RETURN:
*       MV_BAD_PARAM if base address is invalid parameter or target is 
*       unknown.
*
*******************************************************************************/
MV_STATUS mvCtrlAddrDecToParams(MV_DEC_WIN *pAddrDecWin, 
                                MV_DEC_WIN_PARAMS *pWinParam)
{
	MV_TARGET_ATTRIB targetAttrib;
	MV_U32 size;        /* Holds BAR size              */
    /* 2) Initialize Size register                                          */
    /* 2.1 Get address decode size register value to given size             */
    size = ctrlSizeToReg(pAddrDecWin->addrWin.size, BAR_SIZE_ALIGNMENT);   
    /* Size parameter validity check.                                       */
    if (-1 == size)
	{
		DB(mvOsPrintf("mvCtrlAddrDecToParams: ERR. addrDecSizeToReg failed.\n"));
        return MV_BAD_PARAM;
	}

    /* Write the size register value */
    pWinParam->size = size;   
    /* Base address */
    pWinParam->baseAddr = pAddrDecWin->addrWin.baseLow;
	/* attrib and targetId */
	mvCtrlAttribGet(pAddrDecWin->target, &targetAttrib);
	pWinParam->attrib = targetAttrib.attrib;
	pWinParam->targetId = targetAttrib.targetId;
    return MV_OK;

}

/*******************************************************************************
* mvCtrlParamsToAddrDec - Extract address decode struct from registers.
*
* DESCRIPTION:
*       This function extract address decode struct from address decode 
*       registers given as parameters.
*
* INPUT:
*       pWinParam   - Address decode register parameters
*
* OUTPUT:
*       pAddrDecWin - Target window data structure.
*
* RETURN:
*		MV_BAD_PARAM if address decode registers data is invalid.
*
*******************************************************************************/
MV_STATUS mvCtrlParamsToAddrDec(MV_DEC_WIN_PARAMS *pWinParam, 
                                MV_DEC_WIN *pAddrDecWin)
{
	MV_TARGET_ATTRIB targetAttrib;

	pAddrDecWin->addrWin.baseLow = pWinParam->baseAddr;
	
	/* Upper 32bit address base is supported under PCI High Address remap */
	pAddrDecWin->addrWin.baseHigh = 0;	

	/* Prepare sizeReg to ctrlRegToSize function */
	pAddrDecWin->addrWin.size = ctrlRegToSize(pWinParam->size, BAR_SIZE_ALIGNMENT);

	if (-1 == pAddrDecWin->addrWin.size)
	{
		DB(mvOsPrintf("mvCtrlParamsToAddrDec: ERR. ctrlRegToSize failed.\n"));
		return MV_BAD_PARAM;
	}

	/* attrib and targetId */
	targetAttrib.attrib = pWinParam->attrib;
	targetAttrib.targetId = pWinParam->targetId;
	pAddrDecWin->target = mvCtrlTargetGet(&targetAttrib);
	if (MAX_TARGETS == pAddrDecWin->target) 
	{		
		DB(mvOsPrintf("mvCtrlParamsToAddrDec: ERR. 0x%x, 0x%x\n",targetAttrib.targetId, targetAttrib.attrib));
		return MV_ERROR; 
	}
	return MV_OK;
}


/*******************************************************************************
* mvCtrlAttribGet - 
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*
* RETURN:
*
*******************************************************************************/

MV_STATUS mvCtrlAttribGet(MV_TARGET target, MV_TARGET_ATTRIB *targetAttrib)
{

	targetAttrib->attrib = mvTargetDefaultsArray[target].attrib;
	targetAttrib->targetId = mvTargetDefaultsArray[target].targetId;

	return MV_OK;

}
/*******************************************************************************
* mvCtrlTargetGet - 
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*
* RETURN:
*
*******************************************************************************/
MV_TARGET mvCtrlTargetGet(MV_TARGET_ATTRIB *targetAttrib)
{
	MV_TARGET target;
	for (target = SDRAM_CS0; target < MAX_TARGETS ; target ++)
	{
		if ((mvTargetDefaultsArray[target].attrib == targetAttrib->attrib) &&
			(mvTargetDefaultsArray[target].targetId == targetAttrib->targetId))
		{
			/* found it */
			break;
		}
	}

	return target;
}

