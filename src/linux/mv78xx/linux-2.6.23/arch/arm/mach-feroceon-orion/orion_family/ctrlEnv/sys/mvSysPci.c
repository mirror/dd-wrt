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

#include "ctrlEnv/sys/mvSysPci.h"

/* PCI BARs registers offsets are inconsecutive. This struct describes BAR	*/
/* register offsets	and its function where its is located.			*/
/* Also, PCI address remap registers offsets are inconsecutive. This struct	*/
/* describes address remap register offsets					*/
typedef struct _pciBarRegInfo
{
	MV_U32 funcNum;
	MV_U32 baseLowRegOffs;
	MV_U32 baseHighRegOffs;
	MV_U32 sizeRegOffs;
	MV_U32 remapLowRegOffs;
	MV_U32 remapHighRegOffs;
}PCI_BAR_REG_INFO;

typedef struct _pciBarStatus
{
	MV_PCI_BAR	bar;
	int		enable;
}PCI_BAR_STATUS;

PCI_BAR_STATUS pciBarStatusMap[] = 
{
#if defined(MV_INCLUDE_SDRAM_CS0)
	{CS0_BAR, EN},      
#endif
#if defined(MV_INCLUDE_SDRAM_CS1)
        {CS1_BAR, EN}, 	
#endif	      		
#if defined(MV_INCLUDE_SDRAM_CS2)
        {CS2_BAR, EN}, 	
#endif
#if defined(MV_INCLUDE_SDRAM_CS3)
        {CS3_BAR, EN},   		
#endif
#if defined(MV_INCLUDE_DEVICE_CS0)          		
	{DEVCS0_BAR, EN},    	
#endif
#if defined(MV_INCLUDE_DEVICE_CS1)          		
	{DEVCS1_BAR, EN},    	
#endif
#if defined(MV_INCLUDE_DEVICE_CS2)          		
	{DEVCS2_BAR, EN},    	
#endif
	{BOOTCS_BAR, EN},     	
	{MEM_INTER_REGS_BAR, EN},
	{IO_INTER_REGS_BAR, EN}, 
	{P2P_MEM0, DIS},     		
	{P2P_IO, DIS},
	{TBL_TERM, TBL_TERM}       	
};

/* PCI BAR table. Note that table entry number must match its target 		*/
/* enumerator. For example, table entry '4' must describe Deivce CS0 		*/
/* target which is represent by DEVICE_CS0 enumerator (4).                  */
#if 0
MV_PCI_BAR_WIN pciBarMap[] = 
{
/*     base low      base high      size        enable/disable				*/
	{{SDRAM_CS0_BASE , 0, SDRAM_CS0_SIZE      	 },   EN},          
	{{SDRAM_CS1_BASE , 0, SDRAM_CS1_SIZE      	 },   EN},          
	{{SDRAM_CS2_BASE , 0, SDRAM_CS2_SIZE      	 },   EN},          
	{{SDRAM_CS3_BASE , 0, SDRAM_CS3_SIZE      	 },   EN},          
	{{DEVICE_CS0_BASE, 0, DEVICE_CS0_SIZE     	 },   EN},          
	{{DEVICE_CS1_BASE, 0, DEVICE_CS1_SIZE     	 },   EN},          
	{{DEVICE_CS2_BASE, 0, DEVICE_CS2_SIZE     	 },   EN},          
	{{BOOTDEV_CS_BASE, 0, BOOTDEV_CS_SIZE     	 },   EN},          
	{{INTER_REGS_BASE, 0, INTER_REGS_SIZE 	 	 },   EN},
	{{INTER_REGS_BASE, 0, INTER_REGS_SIZE 	 	 },   EN},
	{{  0xFFFFFFFF   , 0,    0xFFFFFFFF          },  DIS}, 	/* Ignore P2P 	*/ 
	{{  0xFFFFFFFF   , 0,    0xFFFFFFFF          },  DIS},	/* Ignore P2P 	*/ 
    /* Table terminator */
    {{TBL_TERM, TBL_TERM, TBL_TERM}, TBL_TERM} 
};
#endif

/* Locals */
static MV_U32 pciBurstBytes2Reg(MV_U32 size);
static MV_U32 pciBurstReg2Bytes(MV_U32 size);

static MV_STATUS pciWinOverlapDetect(MV_U32 pciIf, MV_PCI_BAR bar,
									 MV_ADDR_WIN *pAddrWin);

static MV_STATUS pciBarRegInfoGet(MV_U32 pciIf, MV_PCI_BAR bar, 
								  PCI_BAR_REG_INFO *pBarRegInfo);

static MV_STATUS pciWinIsValid(MV_U32 baseLow, MV_U32 size);

/* Forward declarations */
const MV_8* pciBarNameGet(MV_PCI_BAR bar);

static MV_TARGET pciBarToTarget(MV_PCI_BAR bar)
{
    switch(bar)
    {
	#if defined(MV_INCLUDE_SDRAM_CS0)
	case CS0_BAR:
			return SDRAM_CS0;     
	#endif
	#if defined(MV_INCLUDE_SDRAM_CS1)
	case CS1_BAR:
			return SDRAM_CS1;     
	#endif
	#if defined(MV_INCLUDE_SDRAM_CS2)
	case CS2_BAR:
			return SDRAM_CS2;     
	#endif
	#if defined(MV_INCLUDE_SDRAM_CS3)
	case CS3_BAR:
			return SDRAM_CS3;     
	#endif
	#if defined(MV_INCLUDE_DEVICE_CS0)          		
	case DEVCS0_BAR:
			return DEVICE_CS0;    	
	#endif
	#if defined(MV_INCLUDE_DEVICE_CS1)          		
	case DEVCS1_BAR:
			return DEVICE_CS1;    	
	#endif
	#if defined(MV_INCLUDE_DEVICE_CS2)          		
	case DEVCS2_BAR:
			return DEVICE_CS2;    	
	#endif
	case BOOTCS_BAR:
			return  DEV_BOOCS;    	
	case MEM_INTER_REGS_BAR:
	case IO_INTER_REGS_BAR:
			return  INTER_REGS;
	
 	default:
		mvOsPrintf("pciBarToTarget: ERR. no such target\n");
    }

	return -1;

}
/*******************************************************************************
* mvPciInit - Initialize PCI interfaces
*
* DESCRIPTION:
*       This function initiate the PCI interface:
*       1) Set local bus number. In case of convential PCI it gets the bus
*          number using mvPciLocalBusNumGet(). In case of PCI-X this 
*          information is read only.
*       2) Interface device number. In case of conventional PCI it gets the
*          device number using mvPciLocalDevNumGet(). In case of PCI-X this 
*          information is read only.
*       3) PCI Arbiter if needed.
*       4) Enable Master and Slave on PCI interfaces.
*	5) Open PCI BARs according to default setting. 
*	   Note that PCI bridge (P2P) is NOT initialized.
*	6) Enable CPU to PCI ordering.
*
* INPUT:
*
*       pciIf   - PCI interface number.
*		localBus - Local Bus of the PCI interface to be set
*		localDev - Local Dev of the PCI interface to be set
*		bFirstCall - Indicates wether this is the first call of this
*					 function .
*
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK if function success otherwise MV_ERROR or MV_BAD_PARAM
*
*******************************************************************************/
MV_STATUS mvPciInit(MV_U32 pciIf, MV_PCI_MOD pciIfmod)
{
	MV_PCI_BAR     bar, barix;
	MV_PCI_BAR_WIN dramDecWin;
	MV_PCI_MODE    pciMode;
	MV_CPU_DEC_WIN addrDecWin;
	MV_PCI_PROT_WIN pciProtWin;
   	MV_PCI_BAR_WIN pciBarMap[PCI_MAX_BARS];  
	/* Parameter checking  */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciInit: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_BAD_PARAM;
	}

	/* device and bus numbers */
	if (MV_OK != mvPciModeGet(pciIf, &pciMode))
	{
		mvOsPrintf("mvPciInit: ERR. mvPciModeGet failed\n");
		return MV_ERROR;
	}

	/* First disable all PCI target windows  */
	for (bar = 0; bar < PCI_MAX_BARS; bar++)
    	{
		mvPciTargetWinEnable(pciIf, bar, MV_FALSE);
	}

	/* WA CQ 4382*/
	MV_REG_BIT_SET(PCI_BASE_ADDR_ENABLE_REG(pciIf) ,BIT15);
	
	/* Building in run time the pci bar mapping table */
	for (bar = 0; bar < PCI_MAX_BARS; bar++)
    	{
		for(barix = 0 ;barix < PCI_MAX_BARS; barix++)
		{
			if(pciBarStatusMap[barix].bar == bar)
			{
				pciBarMap[bar].enable = pciBarStatusMap[barix].enable;
				break;
			}
		}

		if(bar == MEM_INTER_REGS_BAR || bar == IO_INTER_REGS_BAR)
		{
			pciBarMap[bar].addrWin.baseLow = mvCpuIfTargetWinBaseLowGet(pciBarToTarget(bar));
			pciBarMap[bar].addrWin.baseHigh = mvCpuIfTargetWinBaseHighGet(pciBarToTarget(bar));
			pciBarMap[bar].addrWin.size = mvCpuIfTargetWinSizeGet(pciBarToTarget(bar));
			continue;
		}

		if(bar == P2P_MEM0 || bar == P2P_IO)
		{
			pciBarMap[bar].addrWin.baseLow = 0xFFFFFFFF;
			pciBarMap[bar].addrWin.baseHigh = 0;
			pciBarMap[bar].addrWin.size = 0xFFFFFFFF;
			continue;
		}

		if (mvCpuIfTargetWinGet(pciBarToTarget(bar), &addrDecWin) == MV_OK)
		{
			pciBarMap[bar].addrWin.baseLow = addrDecWin.addrWin.baseLow;
			pciBarMap[bar].addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
			pciBarMap[bar].addrWin.size = addrDecWin.addrWin.size;
			if(addrDecWin.enable == MV_FALSE) 
			{
				pciBarMap[bar].enable = DIS;
			}
		}
		else
		{
			pciBarMap[bar].addrWin.baseLow = 0xFFFFFFFF;
			pciBarMap[bar].addrWin.baseHigh = 0;
			pciBarMap[bar].addrWin.size = 0xFFFFFFFF;
			pciBarMap[bar].enable = DIS;
		}
	}

	/* finally fill table with TBL_TERM entry */
	bar = PCI_MAX_BARS - 1;
	pciBarMap[bar].addrWin.baseLow = TBL_TERM;
	pciBarMap[bar].addrWin.baseHigh = TBL_TERM;
	pciBarMap[bar].addrWin.size = TBL_TERM;
	pciBarMap[bar].enable =  TBL_TERM;


    	/* Memory Mapped Internal Registers BAR can not be disabled.            */
    	/* Relocate its BAR first to avoid colisions with other BARs (e.g DRAM) */
    	if (MV_OK != mvPciTargetWinSet(pciIf, MEM_INTER_REGS_BAR, 
                                                &pciBarMap[MEM_INTER_REGS_BAR]))
    	{
        	mvOsPrintf("mvPciInit: ERR. mvPciTargetWinSet failed\n");
        	return MV_ERROR;
    	}        

	/* Now, go through all targets in default table until table terminator	*/
	for (bar = 0; pciBarMap[bar].enable != TBL_TERM; bar++)
    	{
		/* Skip the P2P BARs. They should be configured seperately			*/
		if (0xFFFFFFFF == pciBarMap[bar].addrWin.baseLow)
		{
            		continue;
		}
		

		/* check if the size passed is zero ! */
		if (0 == pciBarMap[bar].addrWin.size)
		{
			/* disable the bar */
			mvPciTargetWinEnable(pciIf,bar,MV_FALSE);
			continue;
		}

		/* Get DRAM parameters from CPU interface 							*/
		if (MV_PCI_BAR_IS_DRAM_BAR(bar))
        	{
			if (MV_OK != mvCpuIfTargetWinGet(MV_PCI_DRAM_BAR_TO_DRAM_TARGET(bar),
											 &addrDecWin))
			{
				mvOsPrintf("mvPciInit:ERR. targetWinGet %d fail\n", bar);
				return MV_ERROR;
			}

            		dramDecWin.addrWin.baseLow  = addrDecWin.addrWin.baseLow;
            		dramDecWin.addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
            		dramDecWin.addrWin.size     = addrDecWin.addrWin.size;
			dramDecWin.enable           = addrDecWin.enable;
			
			if (MV_OK != mvPciTargetWinSet(pciIf, bar, &dramDecWin))
			{
				mvOsPrintf("mvPciInit: ERR. mvPciTargetWinSet %d failed\n",bar);
				return MV_ERROR;
			}        

			continue;
       		 }
		
		if (MV_OK != mvPciTargetWinSet(pciIf, bar, &pciBarMap[bar]))
		{
			mvOsPrintf("mvPciInit: ERR. mvPciTargetWinSet %d failed\n", bar);
			return MV_ERROR;
		}        
    	}
	
	MV_REG_BIT_SET(PCI_ADDR_DECODE_CONTROL_REG(pciIf),
				   PADCR_REMAP_REG_WR_DIS);
		

	/* configure access control unit 0 to DDR to enhance performance */
	pciProtWin.addrWin.baseLow = 0;
	pciProtWin.addrWin.baseHigh = 0;
	pciProtWin.addrWin.size = mvDramIfSizeGet();
	pciProtWin.attributes.access = ALLOWED;
	pciProtWin.attributes.write = ALLOWED;
	pciProtWin.attributes.swapType = MV_BYTE_SWAP;
	pciProtWin.attributes.readMaxBurst = 128; 
	pciProtWin.attributes.readBurst = 256;
	pciProtWin.attributes.writeMaxBurst = 128;
	pciProtWin.attributes.pciOrder = MV_FALSE;
	pciProtWin.enable = MV_TRUE;
	if( mvPciProtWinSet(pciIf, 0, &pciProtWin) != MV_OK )
	{
		mvOsPrintf("mvPciInit: ERR. mvPciProtWinSet failed\n");
		return MV_ERROR;
	}

	mvPciHalInit(pciIf, pciIfmod);

	return MV_OK;
}



/*******************************************************************************
* mvPciTargetWinSet - Set PCI to peripheral target address window BAR
*
* DESCRIPTION:
*       This function sets an address window from PCI to a peripheral 
*       target (e.g. SDRAM bank0, PCI_MEM0), also known as BARs. 
*       A new PCI BAR window is set for specified target address window.
*       If address decode window parameter structure enables the window, 
*       the routine will also enable the target window, allowing PCI to access
*       the target window.
*
* INPUT:
*       pciIf       - PCI interface number.
*       bar         - BAR to be accessed by slave.
*       pAddrBarWin - PCI target window information data structure.
*
* OUTPUT:
*       N/A
*
* RETURN:
*       MV_OK if PCI BAR target window was set correctly, MV_BAD_PARAM on bad params 
*       MV_ERROR otherwise 
*       (e.g. address window overlapps with other active PCI target window).
*
*******************************************************************************/
MV_STATUS mvPciTargetWinSet(MV_U32 pciIf,
							MV_PCI_BAR bar, 
                            MV_PCI_BAR_WIN *pAddrBarWin)
{
	MV_U32 pciData;
	MV_U32 sizeToReg;
	MV_U32 size;
	MV_U32 baseLow;
	MV_U32 baseHigh;
	MV_U32 localBus;
	MV_U32 localDev;
	PCI_BAR_REG_INFO barRegInfo;

	size     = pAddrBarWin->addrWin.size;
	baseLow  = pAddrBarWin->addrWin.baseLow;
	baseHigh = pAddrBarWin->addrWin.baseHigh;

	/* Parameter checking   */
	if(pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciTargetWinSet: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_BAD_PARAM;
	}

	if(bar >= PCI_MAX_BARS )
	{
		mvOsPrintf("mvPciTargetWinSet: ERR. Illigal PCI BAR %d\n", bar);
		return MV_BAD_PARAM;
	}


	/* if the address windows is disabled , we only disable the appropriare
	pci bar and ignore other settings */

	if (MV_FALSE == pAddrBarWin->enable)
	{
        MV_REG_BIT_SET(PCI_BASE_ADDR_ENABLE_REG(pciIf), BARER_ENABLE(bar));
		return MV_OK;
	}

	if (0 == pAddrBarWin->addrWin.size)
	{
        mvOsPrintf("mvPciTargetWinSet: ERR. Target %d can't be zero!\n",bar);
        return MV_BAD_PARAM;
	}

	/* Check if the window complies with PCI spec							*/
	if (MV_TRUE != pciWinIsValid(baseLow, size))
	{
        mvOsPrintf("mvPciTargetWinSet: ERR. Target %d window invalid\n", bar);
		return MV_BAD_PARAM;
	}

    /* 2) Check if the requested window overlaps with current windows		*/
	if(MV_TRUE == pciWinOverlapDetect(pciIf, bar, &pAddrBarWin->addrWin))
	{
		mvOsPrintf("mvPciTargetWinSet: ERR. Overlap detected for target %d\n",
																		bar);
		return MV_BAD_PARAM;
	}

	/* Get size register value according to window size						*/
	sizeToReg = ctrlSizeToReg(size, PBBLR_BASE_ALIGNMET);

	/* Size parameter validity check.                                   */
	if (-1 == sizeToReg)
	{
		mvOsPrintf("mvPciTargetWinSet: ERR. Target BAR %d size invalid.\n",bar);
		return MV_BAD_PARAM;
	}

	localBus = mvPciLocalBusNumGet(pciIf);
	localDev = mvPciLocalDevNumGet(pciIf);
	
	/* Get BAR register information */
	pciBarRegInfoGet(pciIf, bar, &barRegInfo);
	
	/* Internal register space size have no size register. Do not perform	*/
	/* size register assigment for this slave target					 	*/
	if (0 != barRegInfo.sizeRegOffs)
	{    
		/* Update size register */
		MV_REG_WRITE(barRegInfo.sizeRegOffs, (sizeToReg << BAR_SIZE_OFFS));
	}
	
	/* Read current address */
	pciData = mvPciConfigRead(pciIf, localBus, localDev, barRegInfo.funcNum, 
													barRegInfo.baseLowRegOffs);

	/* Clear current address */
	pciData &= ~PBBLR_BASE_MASK;
	pciData |= (baseLow & PBBLR_BASE_MASK);
		
	/* Write new address */
	mvPciConfigWrite(pciIf, localBus, localDev, barRegInfo.funcNum,
											barRegInfo.baseLowRegOffs, pciData);

	/* Skip base high settings if the BAR has only base low (32-bit)		*/
	if (0 != barRegInfo.baseHighRegOffs)
	{
		mvPciConfigWrite(pciIf, localBus, localDev, barRegInfo.funcNum, 
										barRegInfo.baseHighRegOffs, baseHigh);	
	}

	/* Enable/disable the BAR */
    if (MV_TRUE == pAddrBarWin->enable)
    {
        MV_REG_BIT_RESET(PCI_BASE_ADDR_ENABLE_REG(pciIf), BARER_ENABLE(bar));
    }
	else
	{
        MV_REG_BIT_SET(PCI_BASE_ADDR_ENABLE_REG(pciIf), BARER_ENABLE(bar));
	}

	return MV_OK;
}

/*******************************************************************************
* mvPciTargetWinGet - Get PCI to peripheral target address window
*
* DESCRIPTION:
*		Get the PCI to peripheral target address window BAR.
*
* INPUT:
*       pciIf - PCI interface number.
*       bar   - BAR to be accessed by slave.
*
* OUTPUT:
*       pAddrBarWin - PCI target window information data structure.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciTargetWinGet(MV_U32 pciIf, MV_PCI_BAR bar, 
                            MV_PCI_BAR_WIN *pAddrBarWin)
{
	MV_U32 size;
	MV_U32 baseLow;
	MV_U32 baseHigh;
	MV_U32 localBus;
	MV_U32 localDev;
	MV_U32 barEnable;
	PCI_BAR_REG_INFO barRegInfo;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciTargetWinGet: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_BAD_PARAM;
	}

	if (bar >= PCI_MAX_BARS )
	{
		mvOsPrintf("mvPciTargetWinGet: ERR. Illigal PCI BAR %d.\n", bar);
		return MV_BAD_PARAM;
	}

	localBus = mvPciLocalBusNumGet(pciIf);
	localDev = mvPciLocalDevNumGet(pciIf);

	/* Get BAR register information */
	pciBarRegInfoGet(pciIf, bar, &barRegInfo);

	/* Reading Base Low bar */
	baseLow = mvPciConfigRead(pciIf, localBus, localDev, barRegInfo.funcNum, 
												barRegInfo.baseLowRegOffs);

	baseLow &= PBBLR_BASE_MASK;

	/* Skip base high if the BAR has only base low (32-bit)		*/
	if (0 != barRegInfo.baseHighRegOffs)
	{
		/* Reading Base High */
		baseHigh = mvPciConfigRead(pciIf, localBus, localDev, barRegInfo.funcNum, 
												barRegInfo.baseHighRegOffs);
	}
	else
	{
		baseHigh = 0;
	}

    /* Internal register space size have no size register. Do not perform	*/
	/* size register assigment for this slave target					 	*/
	if (0 != barRegInfo.sizeRegOffs)
	{    
		/* Reading bar size*/
		size = ctrlRegToSize(
					(MV_REG_READ(barRegInfo.sizeRegOffs) >> PBSR_SIZE_OFFS), 
														PBBLR_BASE_ALIGNMET);
	}
	else
	{
		size = INTER_REGS_SIZE;
	}

	/* Assign value to user struct */
	pAddrBarWin->addrWin.baseLow  = baseLow;
	pAddrBarWin->addrWin.baseHigh = baseHigh;
	pAddrBarWin->addrWin.size     = size;

	/* Check if window is enabled   */
	barEnable = MV_REG_READ(PCI_BASE_ADDR_ENABLE_REG(pciIf));
	
	if (~barEnable & (BARER_ENABLE(bar)))
    {
        pAddrBarWin->enable = MV_TRUE;
    }
    else
    {
        pAddrBarWin->enable = MV_FALSE;
    }

	return MV_OK;
}


/*******************************************************************************
* mvPciTargetWinEnable - Enable/disable a PCI BAR window
*
* DESCRIPTION:
*       This function enable/disable a PCI BAR window.
*       if parameter 'enable' == MV_TRUE the routine will enable the 
*       window, thus enabling PCI accesses for that BAR (before enabling the 
*       window it is tested for overlapping). Otherwise, the window will 
*       be disabled.
*
* INPUT:
*       pciIf  - PCI interface number.
*       bar    - BAR to be accessed by slave.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciTargetWinEnable(MV_U32 pciIf, MV_PCI_BAR bar, MV_BOOL enable)
{
	MV_PCI_BAR_WIN barWin;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciTargetWinEnable: ERR. Invalid PCI interface %d\n",
                                                                        pciIf);
		return MV_BAD_PARAM;
	}

	if (bar >= PCI_MAX_BARS )
	{
		mvOsPrintf("mvPciTargetWinEnable: ERR. Illigal PCI BAR %d\n", bar);
		return MV_BAD_PARAM;
	}
	
	if (MV_TRUE == enable)
	{   /* First check for overlap with other enabled windows				*/
        /* Get current window */
		if (MV_OK != mvPciTargetWinGet(pciIf, bar, &barWin))
		{
			mvOsPrintf("mvPciTargetWinEnable: ERR. targetWinGet fail\n");
			return MV_ERROR;
		}

		/* Check for overlapping */
		if (MV_TRUE == pciWinOverlapDetect(pciIf, bar, &barWin.addrWin))
		
		{   /* Overlap detected	*/
			mvOsPrintf("mvPciTargetWinEnable: ERR. Overlap detected\n");
			return MV_ERROR;
		}
		else
		{
			/* No Overlap. Enable address decode target window              */
			MV_REG_BIT_RESET(PCI_BASE_ADDR_ENABLE_REG(pciIf),BARER_ENABLE(bar));
		}
	}
	else
	{
		/* Disable address decode target window                             */
		MV_REG_BIT_SET(PCI_BASE_ADDR_ENABLE_REG(pciIf), BARER_ENABLE(bar));
	}

	return MV_OK;
}


/*******************************************************************************
* mvPciProtWinSet - Set PCI protection access window
*
* DESCRIPTION:
*       This function sets a specified address window with access protection 
*       attributes. If protection structure enables the window the routine will
*       also enable the protection window.
*
* INPUT:
*       pciIf    - PCI interface number.
*       winNum   - Protecion window number.
*       pProtWin - Protection window structure.
*
* OUTPUT:
*       N/A
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciProtWinSet(MV_U32 pciIf, 
						  MV_U32 winNum, 
                          MV_PCI_PROT_WIN *pProtWin)
{
	MV_U32 protBaseLow;
	MV_U32 protBaseHigh;
	MV_U32 protSize;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciProtWinSet: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_BAD_PARAM;
	}
	if (winNum >= PCI_MAX_PROT_WIN)
	{
		mvOsPrintf("mvPciProtWinSet: ERR. Invalid window num %d\n", winNum);
		return MV_BAD_PARAM;
	}

	/* Check if the window complies with PCI spec							*/
	if (MV_TRUE != pciWinIsValid(pProtWin->addrWin.baseLow, 
                                 pProtWin->addrWin.size))
	{
        mvOsPrintf("mvPciProtWinSet: ERR. Win base 0x%x unaligned to size 0x%x\n",
                   pProtWin->addrWin.baseLow, pProtWin->addrWin.size);

		return MV_BAD_PARAM;
	}

	if (pProtWin->attributes.swapType >= SWAP_TYPE_MAX)
	{
		mvOsPrintf("mvPciProtWinSet: ERR. Swap parameter invalid %d\n",
					                            pProtWin->attributes.swapType);
		return MV_BAD_PARAM;

	}

	/* 1) Calculate protection window base low register value	*/
	protBaseLow  =  pProtWin->addrWin.baseLow;

	/* Setting the appropriate bits according to the passed values */
	if (MV_TRUE == pProtWin->enable) 
	{
		protBaseLow |= PACBLR_EN;
	}
	else
	{
		protBaseLow &= ~PACBLR_EN;
	}


	/* Access protect */
	if (ALLOWED == pProtWin->attributes.access)
	{
		protBaseLow &= ~PACBLR_ACCPROT;
	}
	else
	{
		protBaseLow |= PACBLR_ACCPROT;
	}

	/* Write Protect */
	if (ALLOWED == pProtWin->attributes.write)
	{
		protBaseLow &= ~PACBLR_WRPROT;
	}
	else
	{
		protBaseLow |= PACBLR_WRPROT;
	}
	
	/* PCI slave Data Swap Control */
	protBaseLow |= (pProtWin->attributes.swapType << PACBLR_PCISWAP_OFFS);


	/* Read Max Burst */
	if (( pciBurstBytes2Reg(pProtWin->attributes.readMaxBurst) << PACBLR_RDMBURST_OFFS) > PACBLR_RDMBURST_128BYTE)
	{
		mvOsPrintf("mvPciProtWinSet: ERR illigal read max burst\n");
		return MV_ERROR;
	}
	protBaseLow |= (pciBurstBytes2Reg(pProtWin->attributes.readMaxBurst) << PACBLR_RDMBURST_OFFS);


	/* Typical PCI read transaction Size. Only valid for PCI conventional */
	if ((pciBurstBytes2Reg(pProtWin->attributes.readBurst) << PACBLR_RDSIZE_OFFS) > PACBLR_RDSIZE_256BYTE )
	{
		mvOsPrintf("mvPciProtWinSet: ERR. illigal read size\n");
		return MV_ERROR;
	}
	protBaseLow |= (pciBurstBytes2Reg(pProtWin->attributes.readBurst) << PACBLR_RDSIZE_OFFS);


	/* 2) Calculate protection window base high register value	*/
	protBaseHigh =  pProtWin->addrWin.baseHigh;

	/* 3) Calculate protection window size register value	*/
	protSize     =  ctrlSizeToReg(pProtWin->addrWin.size, PACSR_SIZE_ALIGNMENT) << PACSR_SIZE_OFFS;
    

	/* Write Max Burst */
	if ((pciBurstBytes2Reg(pProtWin->attributes.writeMaxBurst) << PACSR_WRMBURST_OFFS) > PACSR_WRMBURST_128BYTE )
	{
		mvOsPrintf("mvPciProtWinSet: ERR illigal write max burst\n");
		return MV_ERROR;
	}
	protSize |= (pciBurstBytes2Reg(pProtWin->attributes.writeMaxBurst) << PACSR_WRMBURST_OFFS);

	/* Pci Order */
    if (MV_TRUE == pProtWin->attributes.pciOrder)
	{
		protSize |= PACSR_PCI_ORDERING;
	}
	else
	{
		protSize &= ~PACSR_PCI_ORDERING;
	}
		    
	/* Writing protection window walues into registers */
	MV_REG_WRITE(PCI_ACCESS_CTRL_BASEL_REG(pciIf,winNum), protBaseLow);
	MV_REG_WRITE(PCI_ACCESS_CTRL_BASEH_REG(pciIf,winNum), protBaseHigh);
	MV_REG_WRITE(PCI_ACCESS_CTRL_SIZE_REG(pciIf,winNum),  protSize);

	return MV_OK;
}
/*******************************************************************************
* mvPciProtWinGet - Get PCI protection access window
*
* DESCRIPTION:
*       This function gets a specified address window and access protection 
*       attributes for a specific protection window .
*
* INPUT:
*       pciIf    - PCI interface number.
*       winNum   - Protecion window number.
*       pProtWin - pointer to a Protection window structure.
*
* OUTPUT:
*       pProtWin - Protection window structure.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciProtWinGet(MV_U32 pciIf, 
						  MV_U32 winNum, 
                          MV_PCI_PROT_WIN *pProtWin)
{
	MV_U32 protBaseLow;
	MV_U32 protBaseHigh;
	MV_U32 protSize;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciProtWinGet: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_BAD_PARAM;
	}
	if (winNum >= PCI_MAX_PROT_WIN)
	{
		mvOsPrintf("mvPciProtWinGet: ERR. Invalid window num %d\n", winNum);
		return MV_BAD_PARAM;
	}

	/* Writing protection window walues into registers */
	protBaseLow = MV_REG_READ(PCI_ACCESS_CTRL_BASEL_REG(pciIf,winNum));
	protBaseHigh = MV_REG_READ(PCI_ACCESS_CTRL_BASEH_REG(pciIf,winNum));
	protSize = MV_REG_READ(PCI_ACCESS_CTRL_SIZE_REG(pciIf,winNum));


	/* 1) Get Protection Windows base low 	*/
	pProtWin->addrWin.baseLow = protBaseLow & PACBLR_BASE_L_MASK;

	/* Get the appropriate protection attributes according to register bits*/

	/* Is Windows enabled ? */
	if (protBaseLow & PACBLR_EN)
	{
		pProtWin->enable = MV_TRUE;
	}
	else
	{
		pProtWin->enable = MV_FALSE;
	}


	/* What is access protect ? */
	if (protBaseLow & PACBLR_ACCPROT)
	{
		pProtWin->attributes.access = FORBIDDEN;
	}
	else
	{
		pProtWin->attributes.access = ALLOWED;
	}

	/* Is write protect ? */
	if (protBaseLow & PACBLR_WRPROT)
	{
		pProtWin->attributes.write = FORBIDDEN;
	}
	else
	{
		pProtWin->attributes.write = ALLOWED;
	}


    	/* PCI slave Data Swap Control */
	pProtWin->attributes.swapType = (protBaseLow & PACBLR_PCISWAP_MASK) >> PACBLR_PCISWAP_OFFS;


	/* Read Max Burst */
	pProtWin->attributes.readMaxBurst = pciBurstReg2Bytes((protBaseLow & PACBLR_RDMBURST_MASK) >> PACBLR_RDMBURST_OFFS);

	/* Typical PCI read transaction Size. */
	pProtWin->attributes.readBurst = pciBurstReg2Bytes((protBaseLow & PACBLR_RDSIZE_MASK) >> PACBLR_RDSIZE_OFFS);


	/* window base high register value	*/
	pProtWin->addrWin.baseHigh = protBaseHigh;

	/*Calculate protection window size register value	*/
	pProtWin->addrWin.size = ctrlRegToSize(((protSize & PACSR_SIZE_MASK) >> PACSR_SIZE_OFFS),PACSR_SIZE_ALIGNMENT);


	/* Write Max Burst */
	pProtWin->attributes.writeMaxBurst = pciBurstReg2Bytes((protSize & PACSR_WRMBURST_MASK) >> PACSR_WRMBURST_OFFS);

	/* Pci Order */
	if (protSize & PACSR_PCI_ORDERING)
	{
		pProtWin->attributes.pciOrder = MV_TRUE;
	}
	else
	{
		pProtWin->attributes.pciOrder = MV_FALSE;
	}


	return MV_OK;
}


/*******************************************************************************
* mvPciProtWinEnable - Enable/disable a PCI protection access window
*
* DESCRIPTION:
*       This function enable/disable a PCI protection access window.
*       if parameter 'enable' == MV_TRUE the routine will enable the 
*       protection window, otherwise, the protection window will be disabled.
*
* INPUT:
*       pciIf  - PCI interface number.
*       winNum - Protecion window number.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       N/A
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciProtWinEnable(MV_U32 pciIf, MV_U32 winNum, MV_BOOL enable)
{
	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciProtWinEnable: ERR. Invalid PCI interface %d\n", 
																		pciIf);
		return MV_BAD_PARAM;
	}

	if (winNum >= PCI_MAX_PROT_WIN)
	{
		mvOsPrintf("mvPciProtWinEnable: ERR. Invalid window num %d\n", winNum);
		return MV_BAD_PARAM;
	}

    if (MV_TRUE == enable)
    {
        MV_REG_BIT_SET(PCI_ACCESS_CTRL_BASEL_REG(pciIf,winNum), PACBLR_EN);
    }
	else
	{
        MV_REG_BIT_RESET(PCI_ACCESS_CTRL_BASEL_REG(pciIf,winNum), PACBLR_EN);
	}

	return MV_OK;
}


/*******************************************************************************
* mvPciTargetRemap - Set PCI to target address window remap.
*
* DESCRIPTION:
*       The PCI interface supports remap of the BAR original address window.
*       For each BAR it is possible to define a remap address. For example
*       an address 0x12345678 that hits BAR 0x10 (SDRAM CS[0]) will be modified
*       according to remap register but will also be targeted to the 
*       SDRAM CS[0].
*
* INPUT:
*       pciIf    - PCI interface number.
*       bar      - Peripheral target enumerator accessed by slave.
*       pAddrWin - Address window to be checked.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciTargetRemap(MV_U32 pciIf,
						   MV_PCI_BAR bar,
                           MV_ADDR_WIN *pAddrWin)
{
	PCI_BAR_REG_INFO barRegInfo;
	
	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciTargetRemap: ERR. Invalid PCI interface num %d\n", 
																		pciIf);
		return MV_BAD_PARAM;
	}

	if (MV_IS_NOT_ALIGN(pAddrWin->baseLow, PBARR_REMAP_ALIGNMENT))
	{
		mvOsPrintf("mvPciTargetRemap: Error remapping PCI interface %d bar %s."\
				   "\nAddress 0x%08x is unaligned to size 0x%x.\n",
				   pciIf,
				   pciBarNameGet(bar),
                   pAddrWin->baseLow,
				   pAddrWin->size);
		return MV_ERROR;
	}

	pciBarRegInfoGet(pciIf, bar, &barRegInfo);

	/* Set remap low register value */
	MV_REG_WRITE(barRegInfo.remapLowRegOffs, pAddrWin->baseLow);
	
	/* Skip base high settings if the BAR has only base low (32-bit)		*/
	if (0 != barRegInfo.remapHighRegOffs)
	{
		MV_REG_WRITE(barRegInfo.remapHighRegOffs, pAddrWin->baseHigh);
	}

	return MV_OK;
}

/*******************************************************************************
* pciWinOverlapDetect - Detect address windows overlapping
*
* DESCRIPTION:
*       This function detects address window overlapping of a given address 
*       window in PCI BARs.
*
* INPUT:
*       pAddrWin - Address window to be checked.
*       bar      - BAR to be accessed by slave.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the given address window overlap current address
*       decode map, MV_FALSE otherwise.
*
*******************************************************************************/
static MV_BOOL pciWinOverlapDetect(MV_U32 pciIf,
									 MV_PCI_BAR bar,
									 MV_ADDR_WIN *pAddrWin)
{
	MV_U32 		   barEnableReg;
    MV_U32 		   targetBar;
	MV_PCI_BAR_WIN barAddrWin;

    /* Read base address enable register. Do not check disabled windows		*/
	barEnableReg = MV_REG_READ(PCI_BASE_ADDR_ENABLE_REG(pciIf));
	
	for(targetBar = 0; targetBar < PCI_MAX_BARS; targetBar++)
    {
        /* don't check our target or illegal targets */
        if (targetBar == bar)
        {
            continue;
        }
        
		/* Do not check disabled windows	*/
		if (barEnableReg & (BARER_ENABLE(targetBar)))
		{
			continue;
		}

		/* Get window parameters 	*/
		if (MV_OK != mvPciTargetWinGet(pciIf, targetBar, &barAddrWin))
		{
			mvOsPrintf("pciWinOverlapDetect: ERR. TargetWinGet failed\n");
            return MV_ERROR;
		}
        
		/* skip overlapp detect between MEM_INTER_REGS_BAR and IO_INTER_REGS_BAR*/
		if (((bar == MEM_INTER_REGS_BAR)&&(targetBar == IO_INTER_REGS_BAR)) ||
			((bar == IO_INTER_REGS_BAR)&&(targetBar == MEM_INTER_REGS_BAR)))
		{
			return MV_FALSE;
		}
        else if(MV_TRUE == ctrlWinOverlapTest(pAddrWin, &barAddrWin.addrWin))
		{                    
			mvOsPrintf("pciWinOverlapDetect: BAR %d overlap current %d\n", 
															bar, targetBar);
			return MV_TRUE;           
		}
    }
    
	return MV_FALSE;
}

/*******************************************************************************
* cpuWinIsValid - Check if the given address window is valid
*
* DESCRIPTION:
*		PCI spec restrict BAR base to be aligned to BAR size.
*		This function checks if the given address window is valid.
*
* INPUT:
*       baseLow - 32bit low base address.
*       size    - Window size.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the address window is valid, MV_FALSE otherwise.
*
*******************************************************************************/
static MV_STATUS pciWinIsValid(MV_U32 baseLow, MV_U32 size)
{

	/* PCI spec restrict BAR base to be aligned to BAR size					*/
	if(MV_IS_NOT_ALIGN(baseLow, size))
	{
		return MV_ERROR;
	}
	else
	{
		return MV_TRUE;
	}
}

/*******************************************************************************
* pciBarRegInfoGet - Get BAR register information
*
* DESCRIPTION:
* 		PCI BARs registers offsets are inconsecutive. 
*		This function gets a PCI BAR register information like register offsets
*		and function location of the BAR.
*
* INPUT:
*       pciIf - PCI interface number.
*		bar	  - The PCI BAR in question.	
*
* OUTPUT:
*       pBarRegInfo - BAR register info struct.
*
* RETURN:
*		MV_BAD_PARAM when bad parameters ,MV_ERROR on error ,othewise MV_OK
*
*******************************************************************************/
static MV_STATUS pciBarRegInfoGet(MV_U32 pciIf, 
								  MV_PCI_BAR bar,
								  PCI_BAR_REG_INFO *pBarRegInfo)
{
	switch (bar)
	{
		/* Function 0 Bars */
		#if defined(MV_INCLUDE_SDRAM_CS0)
		case CS0_BAR:      		/* SDRAM chip select 0 bar*/
			pBarRegInfo->funcNum          = 0;
			pBarRegInfo->baseLowRegOffs   = PCI_SCS0_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_SCS0_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs      = PCI_CS0_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_CS0_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		#endif
		#if defined(MV_INCLUDE_SDRAM_CS1)
		case CS1_BAR:      		/* SDRAM chip select 1 bar*/
			pBarRegInfo->funcNum          = 0;
			pBarRegInfo->baseLowRegOffs   = PCI_SCS1_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_SCS1_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs      = PCI_CS1_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_CS1_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		#endif
		case MEM_INTER_REGS_BAR: /* Memory Mapped Internal bar */
			pBarRegInfo->funcNum          = 0;
			pBarRegInfo->baseLowRegOffs   = PCI_INTER_REG_MEM_MAPPED_BASE_ADDR_L;
			pBarRegInfo->baseHighRegOffs  = PCI_INTER_REG_MEM_MAPPED_BASE_ADDR_H;
			pBarRegInfo->sizeRegOffs      = 0;
			pBarRegInfo->remapLowRegOffs  = 0;
			pBarRegInfo->remapHighRegOffs = 0;
			break;
	
		/* Function 1 Bars */
		#if defined(MV_INCLUDE_SDRAM_CS2)
		case CS2_BAR:      		/* SDRAM chip select 2 bar*/
			pBarRegInfo->funcNum          = 1;
			pBarRegInfo->baseLowRegOffs   = PCI_SCS2_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_SCS2_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs      = PCI_CS2_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_CS2_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		#endif
		#if defined(MV_INCLUDE_SDRAM_CS3)
		case CS3_BAR:      		/* SDRAM chip select 3 bar*/
			pBarRegInfo->funcNum		  = 1;
			pBarRegInfo->baseLowRegOffs	  = PCI_SCS3_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_SCS3_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_CS3_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_CS3_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		#endif
		#if defined(MV_INCLUDE_DEVICE_CS0) 
		/* Function 2 Bars */
		case DEVCS0_BAR:     	/* Device chip select 0 bar*/
			pBarRegInfo->funcNum		  = 2;
			pBarRegInfo->baseLowRegOffs	  = PCI_DEVCS0_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_DEVCS0_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_DEVCS0_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_DEVCS0_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		#endif
		#if defined(MV_INCLUDE_DEVICE_CS1) 
		case DEVCS1_BAR:     	/* Device chip select 0 bar*/
			pBarRegInfo->funcNum		  = 2;
			pBarRegInfo->baseLowRegOffs	  = PCI_DEVCS1_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_DEVCS1_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_DEVCS1_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_DEVCS1_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		#endif
		#if defined(MV_INCLUDE_DEVICE_CS2) 
		case DEVCS2_BAR:     	/* Device chip select 0 bar*/
			pBarRegInfo->funcNum		  = 2;
			pBarRegInfo->baseLowRegOffs	  = PCI_DEVCS2_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_DEVCS2_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_DEVCS2_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_DEVCS2_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		#endif
		case BOOTCS_BAR:      	/* Boot device chip select bar*/
			pBarRegInfo->funcNum		  = 3;
			pBarRegInfo->baseLowRegOffs	  = PCI_BOOTCS_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_BOOTCS_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_BOOTCS_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_BOOTCS_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
	
		/* Function 4 Bars */
		case P2P_MEM0:      		/* P2P memory 0 */
			pBarRegInfo->funcNum		  = 4;
			pBarRegInfo->baseLowRegOffs	  = PCI_P2P_MEM0_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_P2P_MEM0_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_P2P_MEM0_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_P2P_MEM0_ADDR_REMAP_LOW_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = PCI_P2P_MEM0_ADDR_REMAP_HIGH_REG(pciIf);
			break;
		case P2P_IO:        		/* P2P IO */
			pBarRegInfo->funcNum		  = 4;
			pBarRegInfo->baseLowRegOffs   = PCI_P2P_IO_BASE_ADDR;
			pBarRegInfo->baseHighRegOffs  = 0;
			pBarRegInfo->sizeRegOffs	  = PCI_P2P_IO_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_P2P_IO_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		case IO_INTER_REGS_BAR: /* IO Mapped Internal bar */
			pBarRegInfo->funcNum		  = 4;
			pBarRegInfo->baseLowRegOffs	  = PCI_INTER_REGS_IO_MAPPED_BASE_ADDR;
			pBarRegInfo->baseHighRegOffs  = 0;
			pBarRegInfo->sizeRegOffs	  = 0;
			pBarRegInfo->remapLowRegOffs  = 0;
			pBarRegInfo->remapHighRegOffs = 0;
			break;
	
	
		default: 
			mvOsPrintf("mvPciTargetWinGet: ERR.non existing target\n");
			return MV_ERROR;

	}

	return MV_OK;
}

/*******************************************************************************
* pciBarNameGet - Get the string name of PCI BAR.
*
* DESCRIPTION:
*		This function get the string name of PCI BAR.
*
* INPUT:
*       bar - PCI bar number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       pointer to the string name of PCI BAR.
*
*******************************************************************************/
const MV_8* pciBarNameGet( MV_PCI_BAR bar )
{
	switch( bar ) 
	{
	#if defined(MV_INCLUDE_SDRAM_CS0)
		case CS0_BAR: 
			return "CS0_BAR..............";
	#endif
	#if defined(MV_INCLUDE_SDRAM_CS1)
		case CS1_BAR: 
			return "CS1_BAR..............";
	#endif
	#if defined(MV_INCLUDE_SDRAM_CS2)
		case CS2_BAR: 
			return "CS2_BAR..............";
	#endif
	#if defined(MV_INCLUDE_SDRAM_CS3)
		case CS3_BAR: 
			return "CS3_BAR..............";
	#endif
	#if defined(MV_INCLUDE_DEVICE_CS0)
		case DEVCS0_BAR: 
			return "DEVCS0_BAR...........";
	#endif
	#if defined(MV_INCLUDE_DEVICE_CS1)
		case DEVCS1_BAR: 
			return "DEVCS1_BAR...........";
	#endif
	#if defined(MV_INCLUDE_DEVICE_CS2)
		case DEVCS2_BAR: 
			return "DEVCS2_BAR...........";
	#endif
		case BOOTCS_BAR: 
			return "BOOTCS_BAR...........";
		case MEM_INTER_REGS_BAR: 
			return "MEM_INTER_REGS_BAR...";
		case IO_INTER_REGS_BAR: 
			return "IO_INTER_REGS_BAR....";
		case P2P_MEM0: 
			return "P2P_MEM0.............";
		case P2P_IO: 
			return "P2P_IO...............";
		default:
			 return "target unknown";
	}
}

/*******************************************************************************
* mvPciAddrDecShow - Print the PCI address decode map (BARs).
*
* DESCRIPTION:
*		This function print the PCI address decode map (BARs).
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
MV_VOID mvPciAddrDecShow(MV_VOID)
{
	MV_PCI_BAR_WIN win;
	MV_PCI_BAR bar;
	MV_U32 pciIf;

	for( pciIf = 0; pciIf < mvCtrlPciMaxIfGet(); pciIf++ )
	{ 
		if (MV_FALSE == mvCtrlPwrClckGet(PCI_UNIT_ID, pciIf)) continue;
		mvOsOutput( "\n" );
		mvOsOutput( "PCI%d:\n", pciIf );
		mvOsOutput( "-----\n" );

		for( bar = 0; bar < PCI_MAX_BARS; bar++ ) 
		{
			memset( &win, 0, sizeof(MV_PCI_BAR_WIN) );

			mvOsOutput( "%s ", pciBarNameGet(bar) );

			if( mvPciTargetWinGet( pciIf, bar, &win ) == MV_OK )
			{
				if( win.enable )
				{
                    mvOsOutput( "base %08x, ", win.addrWin.baseLow );
                    mvSizePrint( win.addrWin.size );
                    mvOsOutput( "\n" );
				}
				else
					mvOsOutput( "disable\n" );
			}
		}
	}	
}

/* convert burst bytes to register value*/
static MV_U32 pciBurstBytes2Reg(MV_U32 size)
{
        MV_U32 ret;
        switch(size)
        {
                case 32: ret = 0; break;
                case 64: ret = 1; break;
                case 128: ret = 2; break;
                case 256: ret = 3; break;
                default: ret = 0xF; /* error */
        }
        return ret;
}

/* convert register value to burst bytes*/
static MV_U32 pciBurstReg2Bytes(MV_U32 size)
{
        MV_U32 ret;
        switch(size)
        {
                case 0: ret = 32; break;
                case 1: ret = 64; break;
                case 2: ret = 128; break;
                case 3: ret = 256; break;
                default: ret = 0x0; /* error */
        }
        return ret;
}

