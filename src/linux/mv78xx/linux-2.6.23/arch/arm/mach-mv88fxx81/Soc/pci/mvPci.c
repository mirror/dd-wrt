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

#if defined(MV_88F5181)
#include "mvPci.h"
#include "mvPciConfig.h"
#include "mvCpuIf.h"
#include "mvIdma.h" /* PCI data parity error workaround */

/* defines  */       
#ifdef MV_DEBUG         
	#define DB(x)	x
#else                
	#define DB(x)    
#endif	             
					 
/* locals */         
/* PCI BARs registers offsets are inconsecutive. This struct describes BAR	*/
/* register offsets	and its function where its is located.					*/
/* Also, PCI address remap registers offsets are inconsecutive. This struct	*/
/* describes address remap register offsets									*/
typedef struct _pciBarRegInfo
{
	MV_U32 funcNum;
	MV_U32 baseLowRegOffs;
	MV_U32 baseHighRegOffs;
	MV_U32 sizeRegOffs;
	MV_U32 remapLowRegOffs;
	MV_U32 remapHighRegOffs;
}PCI_BAR_REG_INFO;

/* PCI BAR table. Note that table entry number must match its target 		*/
/* enumerator. For example, table entry '4' must describe Deivce CS0 		*/
/* target which is represent by DEVICE_CS0 enumerator (4).                  */
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

/* Locals */
static MV_STATUS pciWinOverlapDetect(MV_U32 pciIf, MV_PCI_BAR bar,
									 MV_ADDR_WIN *pAddrWin);

static MV_STATUS pciBarRegInfoGet(MV_U32 pciIf, MV_PCI_BAR bar, 
								  PCI_BAR_REG_INFO *pBarRegInfo);

static MV_STATUS pciWinIsValid(MV_U32 baseLow, MV_U32 size);

static MV_U32 pciBurstBytes2Reg(MV_U32 size);
static MV_U32 pciBurstReg2Bytes(MV_U32 size);

/* Forward declarations */
const MV_8* pciBarNameGet(MV_PCI_BAR bar);

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
*		5) Open PCI BARs according to default setting. 
*		   Note that PCI bridge (P2P) is NOT initialized.
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
MV_STATUS mvPciInit(MV_U32 pciIf)
{
	MV_PCI_BAR     bar;
	MV_PCI_BAR_WIN dramDecWin;
	MV_PCI_MODE    pciMode;
	MV_CPU_DEC_WIN addrDecWin;

    
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
	
    	/* Local bus and device numbers can only be set under conventional PCI	*/
	if (pciIf == 0)
	{
		mvPciLocalBusNumSet(pciIf, PCI0_HOST_BUS_NUM);
		mvPciLocalDevNumSet(pciIf, PCI0_HOST_DEV_NUM);
	}
	else /* PCI1 */
	{
		mvPciLocalBusNumSet(pciIf, PCI1_HOST_BUS_NUM);
		mvPciLocalDevNumSet(pciIf, PCI1_HOST_DEV_NUM);

	}

	/* Local device master Enable */
	mvPciMasterEnable(pciIf, MV_TRUE);

	/* Local device slave Enable */
	mvPciSlaveEnable(pciIf, mvPciLocalBusNumGet(pciIf),
					 mvPciLocalDevNumGet(pciIf), MV_TRUE);


	MV_REG_BIT_SET(PCI_ADDR_DECODE_CONTROL_REG(pciIf),
				   PADCR_REMAP_REG_WR_DIS);
		

	return MV_OK;
}

/*******************************************************************************
* mvPciWaFix - PCI Workaround fix implementation
*
* DESCRIPTION:
*       This function implements PCI restrictions according to PCI/PCI-X and 
*       PCI Express Guidelines and Restrictions Doc. No. MV-S300756-00.
*       This function supports the following Restrictions:
*       - RES# PCI-4, PCI/PCI-X Slave Incorrectly Sets Internal Data 
*         Parity Interrupt 
*       - RES# PCI-5, Master May Erroneously Set/Miss an Internal Data Parity 
*         Interrupt in Conventional PCI
*       
*       For both RES# PCI-4 and RES# PCI-5:
*       1. Prior to enabling the internal data parity interrupt, set the 
*       <Mask> bit[0] in the PCI Interrupt Mask register to 0 (Mask Enabled).
*       2. Use the DMA engine to perform four dummy writes, of 128 bytes each, 
*       from the MV88F5x8x Master toward the PCI. To include RES# PCI-4 the 
*       function activate the Loopback Test mode capability which cause 
*       the slave SRAM to clear.
*       3. Clear the internal data parity interrupt <DPErr> bit[0].
*       4. Set the <Mask> bit[0] in the PCI Interrupt Mask register to 1 
*       (Mask Disabled).
*
* INPUT:
*       pciIf - PCI interface number.
*
* OUTPUT:
*       None.
*
* RETURN:
*	    None
*
*******************************************************************************/
MV_VOID mvPciWaFix(MV_U32 pciIf)
{
	MV_U32	devNumber[6]; /* we have only three slots */
	MV_U32 i;
	MV_U32 localBusNum, localDevNum;
	MV_U32 dev=0,data,devIndex=0;

	
    localDevNum = mvPciLocalDevNumGet(pciIf);
    localBusNum = mvPciLocalBusNumGet(pciIf);
    
    mvPciMasterEnable(pciIf, MV_TRUE);

    mvPciSlaveEnable(pciIf, localBusNum, localDevNum, MV_TRUE);

    /* we will save device number of pci agents that were enabled ,         */
    /* so we can disable them and then enable them later                    */ 

	/* write -0xff to init array */
	for (i=0; i < 6; i++)
	{
		devNumber[i] = 0xff;
	}

	/* first disable all pci agents current on the bus. We do not want      */
    /* them to respond to the dummy writes                                  */
	for (dev=0;dev < 32; dev++)
	{
		if (dev != localDevNum)
		{
			data = mvPciConfigRead(pciIf, localBusNum, dev, 0, 0x4);
			if (((data & PSCR_IO_EN)||(data & PSCR_MEM_EN))&&
				(data != 0xffffffff))
			{	
				devNumber[devIndex++] = dev;
				mvPciSlaveEnable(pciIf, localBusNum, dev, MV_FALSE);
			}
		}
	}	

    /* set <Mask> bit[0] in the PCI Interrupt Mask register to 0 */
    MV_REG_BIT_RESET(PCI_SERRN_MASK_REG(pciIf), 0x1);

	/*enable Loop Back */
    MV_REG_BIT_SET(PCI_CMD_REG(pciIf), PCR_LOOP_BACK_ENABLE);

	mvDmaCtrlLowSet(0, (ICCLR_NON_CHAIN_MODE        | 
                        ICCLR_BLOCK_MODE            |
                        ICCLR_OVRRD_SRC_BAR(2)      | 
                        ICCLR_OVRRD_DST_BAR(2)      |
                        ICCLR_DST_BURST_LIM_128BYTE |   
                        ICCLR_SRC_BURST_LIM_128BYTE ));

    mvDmaTransfer(0, 0x00000000, 0x00000000, 0x400, (MV_U32)NULL);

	while(MV_ACTIVE == mvDmaStateGet(0));

    /*disable Loop Back */
    MV_REG_BIT_RESET(PCI_CMD_REG(pciIf), PCR_LOOP_BACK_ENABLE);

    /* Clear the internal data parity interrupt <DPErr> bit[0]. */
    MV_REG_BIT_RESET(PCI_SERRN_MASK_REG(pciIf), 0x1);

	/* Now re- enable the PCI devices */
	for (dev=0;dev < 6; dev++)
	{
		if (devNumber[dev] != 0xff )
		{
			mvPciSlaveEnable(pciIf, localBusNum, devNumber[dev], MV_TRUE);
		}
	}

	return;
}

/*******************************************************************************
* mvPciCommandSet - Set PCI comman register value.
*
* DESCRIPTION:
*       This function sets a given PCI interface with its command register 
*       value.
*
* INPUT:
*       pciIf   - PCI interface number.
*       command - 32bit value to be written to comamnd register.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM if pciIf is not in range otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciCommandSet(MV_U32 pciIf, MV_U32 command)
{
    MV_U32 locBusNum, locDevNum, regVal;

    locBusNum =  mvPciLocalBusNumGet(pciIf);
    locDevNum =  mvPciLocalDevNumGet(pciIf);

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciCommandSet: ERR. Invalid PCI IF num %d\n", pciIf);
		return MV_BAD_PARAM;
	}

	/* Set command register */
	MV_REG_WRITE(PCI_CMD_REG(pciIf), command);

    /* Support device errata */
    if ((command & PCR_CPU_TO_PCI_ORDER_EN) && 
        (command & PCR_PCI_TO_CPU_ORDER_EN))
    {
        /* Read PCI-X command register */
        regVal = mvPciConfigRead (pciIf, locBusNum, locDevNum, 0, PCIX_COMMAND);
                        
        /* clear bits 22:20 */
        regVal &= 0xff8fffff;

        /* set reset value */
        regVal |= (0x3 << 20);
        
        /* Write back the value */
        mvPciConfigWrite (pciIf, locBusNum, locDevNum, 0, PCIX_COMMAND, regVal);
    }

	return MV_OK;


}


/*******************************************************************************
* mvPciModeGet - Get PCI interface mode.
*
* DESCRIPTION:
*       This function returns the given PCI interface mode.
*
* INPUT:
*       pciIf   - PCI interface number.
*
* OUTPUT:
*       pPciMode - Pointer to PCI mode structure.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciModeGet(MV_U32 pciIf, MV_PCI_MODE *pPciMode)
{
	MV_U32 pciMode;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciModeGet: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_BAD_PARAM;
	}
	if (NULL == pPciMode)
	{
		mvOsPrintf("mvPciModeGet: ERR. pPciMode = NULL  \n");
		return MV_BAD_PARAM;
	}

	/* Read pci mode register */
	pciMode = MV_REG_READ(PCI_MODE_REG(pciIf));
	
	switch (pciMode & PMR_PCI_MODE_MASK)
	{
		case PMR_PCI_MODE_CONV:
            pPciMode->pciType  = MV_PCI_CONV;

			if (MV_REG_READ(PCI_DLL_CTRL_REG(pciIf)) & PDC_DLL_EN)
			{
				pPciMode->pciSpeed = 66000000; /* 66MHZ */
			}
			else
			{
				pPciMode->pciSpeed = 33000000; /* 33MHZ */
			}
			
			break;
		
		case PMR_PCI_MODE_PCIX_66MHZ:	
			pPciMode->pciType  = MV_PCIX;
			pPciMode->pciSpeed = 66000000; /* 66MHZ */	
			break;
		
		case PMR_PCI_MODE_PCIX_100MHZ:	
			pPciMode->pciType  = MV_PCIX;
			pPciMode->pciSpeed = 100000000; /* 100MHZ */	
			break;
		
		case PMR_PCI_MODE_PCIX_133MHZ:	
			pPciMode->pciType  = MV_PCIX;
			pPciMode->pciSpeed = 133000000; /* 133MHZ */	
			break;

		default:
			{
				mvOsPrintf("mvPciModeGet: ERR. Non existing mode !!\n");
				return MV_ERROR;
			}
	}

	switch (pciMode & PMR_PCI_64_MASK)
	{
		case PMR_PCI_64_64BIT:
			pPciMode->pciWidth = MV_PCI_64;
			break;
		
		case PMR_PCI_64_32BIT:
            pPciMode->pciWidth = MV_PCI_32;
            break;
		
		default:
			{
				mvOsPrintf("mvPciModeGet: ERR. Non existing mode !!\n");
				return MV_ERROR;
			}
	}
	
	return MV_OK;
}

/*******************************************************************************
* mvPciRetrySet - Set PCI retry counters
*
* DESCRIPTION:
*       This function specifies the number of times the PCI controller 
*       retries a transaction before it quits.
*       Applies to the PCI Master when acting as a requester.
*       Applies to the PCI slave when acting as a completer (PCI-X mode).
*       A 0x00 value means a "retry forever".
*
* INPUT:
*       pciIf   - PCI interface number.
*       counter - Number of times PCI controller retry. Use counter value 
*                 up to PRR_RETRY_CNTR_MAX.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciRetrySet(MV_U32 pciIf, MV_U32 counter)
{
	MV_U32 pciRetry;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciRetrySet: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_BAD_PARAM;
	}

	if (counter >= PRR_RETRY_CNTR_MAX)
	{
		mvOsPrintf("mvPciRetrySet: ERR. Invalid counter: %d\n", counter);
		return MV_BAD_PARAM;

	}

	/* Reading PCI retry register */
    pciRetry  = MV_REG_READ(PCI_RETRY_REG(pciIf));

	pciRetry &= ~PRR_RETRY_CNTR_MASK;

	pciRetry |= (counter << PRR_RETRY_CNTR_OFFS);

	/* write new value */
	MV_REG_WRITE(PCI_RETRY_REG(pciIf), pciRetry);

	return MV_OK;
}


/*******************************************************************************
* mvPciDiscardTimerSet - Set PCI discard timer
*
* DESCRIPTION:
*       This function set PCI discard timer.
*       In conventional PCI mode:
*       Specifies the number of PCLK cycles the PCI slave keeps a non-accessed
*       read buffers (non-completed delayed read) before invalidate the buffer.
*       Set to '0' to disable the timer. The PCI slave waits for delayed 
*       read completion forever.
*       In PCI-X mode:
*       Specifies the number of PCLK cycles the PCI master waits for split
*       completion transaction, before it invalidates the pre-allocated read
*       buffer.
*       Set to '0' to disable the timer. The PCI master waits for split 
*       completion forever.
*       NOTE: Must be set to a number greater than MV_PCI_MAX_DISCARD_CLK, 
*       unless using the "wait for ever" setting 0x0.
*       NOTE: Must not be updated while there are pending read requests.
*
* INPUT:
*       pciIf      - PCI interface number.
*       pClkCycles - Number of PCI clock cycles.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciDiscardTimerSet(MV_U32 pciIf, MV_U32 pClkCycles)
{
	MV_U32 pciDiscardTimer;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciDiscardTimerSet: ERR. Invalid PCI interface %d\n", 
																		pciIf);
		return MV_BAD_PARAM;
	}

	if (pClkCycles >= PDTR_TIMER_MIN)
	{
		mvOsPrintf("mvPciDiscardTimerSet: ERR. Invalid Clk value: %d\n", 	
																   pClkCycles);
		return MV_BAD_PARAM;

	}

	/* Read  PCI Discard Timer */
	pciDiscardTimer  = MV_REG_READ(PCI_DISCARD_TIMER_REG(pciIf));

	pciDiscardTimer &= ~PDTR_TIMER_MASK;

    pciDiscardTimer |= (pClkCycles << PDTR_TIMER_OFFS);

	/* Write new value */
	MV_REG_WRITE(PCI_DISCARD_TIMER_REG(pciIf), pciDiscardTimer);

	return MV_OK;

}

/* PCI Arbiter routines */

/*******************************************************************************
* mvPciArbEnable - PCI arbiter enable/disable
*
* DESCRIPTION:
*       This fuction enable/disables a given PCI interface arbiter.
*       NOTE: Arbiter setting can not be changed while in work. It should only 
*             be set once.
* INPUT:
*       pciIf  - PCI interface number.
*       enable - Enable/disable parameter. If enable = MV_TRUE then enable.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_STATUS mvPciArbEnable(MV_U32 pciIf, MV_BOOL enable)
{
	MV_U32 regVal;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciArbEnable: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_ERROR;
	}
    	
    /* Set PCI Arbiter Control register according to default configuration 	*/
	regVal = MV_REG_READ(PCI_ARBITER_CTRL_REG(pciIf));

	/* Make sure arbiter disabled before changing its values */
	MV_REG_BIT_RESET(PCI_ARBITER_CTRL_REG(pciIf), PACR_ARB_ENABLE); 

	regVal &= ~PCI_ARBITER_CTRL_DEFAULT_MASK;	

	regVal |= PCI_ARBITER_CTRL_DEFAULT;		/* Set default configuration	*/

	if (MV_TRUE == enable)
	{
		regVal |= PACR_ARB_ENABLE; 
	}
	else
	{
		regVal &= ~PACR_ARB_ENABLE; 
	}

	/* Write to register 										            */
	MV_REG_WRITE(PCI_ARBITER_CTRL_REG(pciIf), regVal);

	return MV_OK;	
}


/*******************************************************************************
* mvPciArbParkDis - Disable arbiter parking on agent
*
* DESCRIPTION:
*       This function disables the PCI arbiter from parking on the given agent
*       list.
*
* INPUT:
*       pciIf        - PCI interface number.
*       pciAgentMask - When a bit in the mask is set to '1', parking on 
*                      the associated PCI master is disabled. Mask bit 
*                      refers to bit 0 - 6. For example disable parking on PCI
*                      agent 3 set pciAgentMask 0x4 (bit 3 is set).
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_STATUS mvPciArbParkDis(MV_U32 pciIf, MV_U32 pciAgentMask)
{
	MV_U32 pciArbiterCtrl;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciArbParkDis: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_ERROR;
	}

	/* Reading Arbiter Control register */
	pciArbiterCtrl = MV_REG_READ(PCI_ARBITER_CTRL_REG(pciIf));

	/* Arbiter must be disabled before changing parking */
	MV_REG_BIT_RESET(PCI_ARBITER_CTRL_REG(pciIf), PACR_ARB_ENABLE); 

	/* do the change */
    pciArbiterCtrl &= ~PACR_PARK_DIS_MASK;
	pciArbiterCtrl |= (pciAgentMask << PACR_PARK_DIS_OFFS);

	/* writing new value ( if th earbiter was enabled before the change		*/ 
	/* here it will be reenabled 											*/
	MV_REG_WRITE(PCI_ARBITER_CTRL_REG(pciIf), pciArbiterCtrl);

	return MV_OK;
}


/*******************************************************************************
* mvPciArbBrokDetectSet - Set PCI arbiter broken detection
*
* DESCRIPTION:
*       This function sets the maximum number of cycles that the arbiter 
*       waits for a PCI master to respond to its grant assertion. If a 
*       PCI agent fails to respond within this time, the PCI arbiter aborts 
*       the transaction and performs a new arbitration cycle.
*       NOTE: Value must be greater than '1' for conventional PCI and 
*       greater than '5' for PCI-X.
*
* INPUT:
*       pciIf      - PCI interface number.
*       pClkCycles - Number of PCI clock cycles. If equal to '0' the broken
*                    master detection is disabled.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciArbBrokDetectSet(MV_U32 pciIf, MV_U32 pClkCycles)
{
	MV_U32 pciArbiterCtrl;
	MV_U32 pciMode;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciArbBrokDetectSet: ERR. Invalid PCI interface %d\n", 
																		pciIf);
		return MV_BAD_PARAM;
	}

	/* Checking PCI mode and if pClkCycles is legal value */
	pciMode = MV_REG_READ(PCI_MODE_REG(pciIf));
	pciMode &= PMR_PCI_MODE_MASK;

	if (PMR_PCI_MODE_CONV == pciMode)
	{
		if (pClkCycles < PACR_BROKEN_VAL_CONV_MIN) 
			return MV_ERROR;
	}
	else
	{
		if (pClkCycles < PACR_BROKEN_VAL_PCIX_MIN) 
			return MV_ERROR;
	}

	pClkCycles <<= PACR_BROKEN_VAL_OFFS;

	/* Reading Arbiter Control register */
	pciArbiterCtrl  = MV_REG_READ(PCI_ARBITER_CTRL_REG(pciIf));
	pciArbiterCtrl &= ~PACR_BROKEN_VAL_MASK;
	pciArbiterCtrl |= pClkCycles;

	/* Arbiter must be disabled before changing broken detection */
	MV_REG_BIT_RESET(PCI_ARBITER_CTRL_REG(pciIf), PACR_ARB_ENABLE); 

	/* writing new value ( if th earbiter was enabled before the change 	*/
	/* here it will be reenabled 											*/

	MV_REG_WRITE(PCI_ARBITER_CTRL_REG(pciIf), pciArbiterCtrl);

	return MV_OK;
}

/* PCI configuration space read write */

/*******************************************************************************
* mvPciConfigRead - Read from configuration space
*
* DESCRIPTION:
*       This function performs a 32 bit read from PCI configuration space.
*       It supports both type 0 and type 1 of Configuration Transactions 
*       (local and over bridge). In order to read from local bus segment, use 
*       bus number retrieved from mvPciLocalBusNumGet(). Other bus numbers 
*       will result configuration transaction of type 1 (over bridge).
*
* INPUT:
*       pciIf   - PCI interface number.
*       bus     - PCI segment bus number.
*       dev     - PCI device number.
*       func    - Function number.
*       regOffs - Register offset.       
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit register data, 0xffffffff on error
*
*******************************************************************************/
MV_U32 mvPciConfigRead (MV_U32 pciIf, MV_U32 bus, MV_U32 dev, MV_U32 func, 
                        MV_U32 regOff)
{
	MV_U32 pciData = 0;

	/* Parameter checking   */
	if (PCI_DEFAULT_IF != pciIf)
	{
		if (pciIf >= mvCtrlPciMaxIfGet())
		{
			mvOsPrintf("mvPciConfigRead: ERR. Invalid PCI interface %d\n",pciIf);
			return 0xFFFFFFFF;
		}
	}

	if (dev >= MAX_PCI_DEVICES)
	{
		DB(mvOsPrintf("mvPciConfigRead: ERR. device number illigal %d\n", dev));
		return 0xFFFFFFFF;
	}
	
	if (func >= MAX_PCI_FUNCS)
	{
		DB(mvOsPrintf("mvPciConfigRead: ERR. function number illigal %d\n", func));
		return 0xFFFFFFFF;
	}
	
	if (bus >= MAX_PCI_BUSSES)
	{
		DB(mvOsPrintf("mvPciConfigRead: ERR. bus number illigal %d\n", bus));
		return MV_ERROR;
	}


	/* Creating PCI address to be passed */
	pciData |= (bus << PCAR_BUS_NUM_OFFS);
	pciData |= (dev << PCAR_DEVICE_NUM_OFFS);
	pciData |= (func << PCAR_FUNC_NUM_OFFS);
	pciData |= (regOff & PCAR_REG_NUM_MASK);

	pciData |= PCAR_CONFIG_EN; 
	
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE(PCI_CONFIG_ADDR_REG(pciIf), pciData);

	/* In order to let the PCI controller absorbed the address of the read 	*/
	/* transaction we perform a validity check that the address was written */
	if(pciData != MV_REG_READ(PCI_CONFIG_ADDR_REG(pciIf)))
	{
		return MV_ERROR;
	}
	/* Read the Data returned in the PCI Data register */
	pciData = MV_REG_READ(PCI_CONFIG_DATA_REG(pciIf));

	return pciData;
}

/*******************************************************************************
* mvPciConfigWrite - Write to configuration space
*
* DESCRIPTION:
*       This function performs a 32 bit write to PCI configuration space.
*       It supports both type 0 and type 1 of Configuration Transactions 
*       (local and over bridge). In order to write to local bus segment, use 
*       bus number retrieved from mvPciLocalBusNumGet(). Other bus numbers 
*       will result configuration transaction of type 1 (over bridge).
*
* INPUT:
*       pciIf   - PCI interface number.
*       bus     - PCI segment bus number.
*       dev     - PCI device number.
*       func    - Function number.
*       regOffs - Register offset.       
*       data    - 32bit data.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciConfigWrite(MV_U32 pciIf, MV_U32 bus, MV_U32 dev, 
                           MV_U32 func, MV_U32 regOff, MV_U32 data)
{
	MV_U32 pciData = 0;

	/* Parameter checking   */
	if (PCI_DEFAULT_IF != pciIf)
	{
		if (pciIf >= mvCtrlPciMaxIfGet())
		{
			mvOsPrintf("mvPciConfigWrite: ERR. Invalid PCI interface %d\n", 
																		pciIf);
			return 0xFFFFFFFF;
		}
	}

	if (dev >= MAX_PCI_DEVICES)
	{
		mvOsPrintf("mvPciConfigWrite: ERR. device number illigal %d\n",dev);
		return MV_BAD_PARAM;
	}

	if (func >= MAX_PCI_FUNCS)
	{
		mvOsPrintf("mvPciConfigWrite: ERR. function number illigal %d\n", func);
		return MV_ERROR;
	}

	if (bus >= MAX_PCI_BUSSES)
	{
		mvOsPrintf("mvPciConfigWrite: ERR. bus number illigal %d\n", bus);
		return MV_ERROR;
	}

	/* Creating PCI address to be passed */
	pciData |= (bus << PCAR_BUS_NUM_OFFS);
	pciData |= (dev << PCAR_DEVICE_NUM_OFFS);
	pciData |= (func << PCAR_FUNC_NUM_OFFS);
	pciData |= (regOff & PCAR_REG_NUM_MASK);

	pciData |= PCAR_CONFIG_EN;
	
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE(PCI_CONFIG_ADDR_REG(pciIf), pciData);

	/* In order to let the PCI controller absorbed the address of the read 	*/
	/* transaction we perform a validity check that the address was written */
	if(pciData != MV_REG_READ(PCI_CONFIG_ADDR_REG(pciIf)))
	{
		return MV_ERROR;
	}

	/* Write the Data passed to the PCI Data register */
	MV_REG_WRITE(PCI_CONFIG_DATA_REG(pciIf), data);

	return MV_OK;
}

/*******************************************************************************
* mvPciMasterEnable - Enable/disale PCI interface master transactions.
*
* DESCRIPTION:
*       This function performs read modified write to PCI command status 
*       (offset 0x4) to set/reset bit 2. After this bit is set, the PCI 
*       master is allowed to gain ownership on the bus, otherwise it is 
*       incapable to do so.
*
* INPUT:
*       pciIf  - PCI interface number.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciMasterEnable(MV_U32 pciIf, MV_BOOL enable)
{
	MV_U32 pciCommandStatus;
	MV_U32 RegOffs;
	MV_U32 localBus;
	MV_U32 localDev;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciMasterEnable: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_ERROR;
	}

	localBus = mvPciLocalBusNumGet(pciIf);
	localDev = mvPciLocalDevNumGet(pciIf);
	
	RegOffs = PCI_STATUS_AND_COMMAND;

	pciCommandStatus = mvPciConfigRead(pciIf, localBus, localDev, 0, RegOffs);

	if (MV_TRUE == enable)
	{
		pciCommandStatus |= PSCR_MASTER_EN;
	}
	else
	{
		pciCommandStatus &= ~PSCR_MASTER_EN;
	}

	mvPciConfigWrite(pciIf, localBus, localDev, 0, RegOffs, pciCommandStatus);

	return MV_OK;
}


/*******************************************************************************
* mvPciSlaveEnable - Enable/disale PCI interface slave transactions.
*
* DESCRIPTION:
*       This function performs read modified write to PCI command status 
*       (offset 0x4) to set/reset bit 0 and 1. After those bits are set, 
*       the PCI slave is allowed to respond to PCI IO space access (bit 0) 
*       and PCI memory space access (bit 1). 
*
* INPUT:
*       pciIf  - PCI interface number.
*       dev     - PCI device number.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciSlaveEnable(MV_U32 pciIf, MV_U32 bus, MV_U32 dev, MV_BOOL enable)
{
	MV_U32 pciCommandStatus;
	MV_U32 RegOffs;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciSlaveEnable: ERR. Invalid PCI interface %d\n", pciIf);
		return MV_BAD_PARAM;
	}
	if (dev >= MAX_PCI_DEVICES)
	{
		mvOsPrintf("mvPciLocalDevNumSet: ERR. device number illigal %d\n", dev);
		return MV_BAD_PARAM;

	}

	RegOffs = PCI_STATUS_AND_COMMAND;
	
	pciCommandStatus=mvPciConfigRead(pciIf, bus, dev, 0, RegOffs);

    if (MV_TRUE == enable)
	{
		pciCommandStatus |= (PSCR_IO_EN | PSCR_MEM_EN);
	}
	else                             
	{
		pciCommandStatus &= ~(PSCR_IO_EN | PSCR_MEM_EN);
	}

	mvPciConfigWrite(pciIf, bus, dev, 0, RegOffs, pciCommandStatus);

	return MV_OK;
}

/*******************************************************************************
* mvPciLocalBusNumSet - Set PCI interface local bus number.
*
* DESCRIPTION:
*       This function sets given PCI interface its local bus number.
*       Note: In case the PCI interface is PCI-X, the information is read-only.
*
* INPUT:
*       pciIf  - PCI interface number.
*       busNum - Bus number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_NOT_ALLOWED in case PCI interface is PCI-X. 
*       MV_BAD_PARAM on bad parameters ,
*       otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciLocalBusNumSet(MV_U32 pciIf, MV_U32 busNum)
{
	MV_U32 pciP2PConfig;
	MV_PCI_MODE pciMode;
	MV_U32 localBus;
	MV_U32 localDev;


	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciLocalBusNumSet: ERR. Invalid PCI interface %d\n",pciIf);
		return MV_BAD_PARAM;
	}
	if (busNum >= MAX_PCI_BUSSES)
	{
		mvOsPrintf("mvPciLocalBusNumSet: ERR. bus number illigal %d\n", busNum);
		return MV_ERROR;

	}

	localBus = mvPciLocalBusNumGet(pciIf);
	localDev = mvPciLocalDevNumGet(pciIf);


	/* PCI interface mode */
	mvPciModeGet(pciIf, &pciMode);

	/* if PCI type is PCI-X then it is not allowed to change the dev number */
	if (MV_PCIX == pciMode.pciType)
	{
		pciP2PConfig = mvPciConfigRead(pciIf, localBus, localDev, 0, PCIX_STATUS );

		pciP2PConfig &= ~PXS_BN_MASK;

		pciP2PConfig |= (busNum << PXS_BN_OFFS) & PXS_BN_MASK;

		mvPciConfigWrite(pciIf, localBus, localDev, 0, PCIX_STATUS,pciP2PConfig );

	}
	else
	{
		pciP2PConfig  = MV_REG_READ(PCI_P2P_CONFIG_REG(pciIf));

		pciP2PConfig &= ~PPCR_BUS_NUM_MASK;

		pciP2PConfig |= (busNum << PPCR_BUS_NUM_OFFS) & PPCR_BUS_NUM_MASK;

		MV_REG_WRITE(PCI_P2P_CONFIG_REG(pciIf), pciP2PConfig);

	}


	return MV_OK;
}


/*******************************************************************************
* mvPciLocalBusNumGet - Get PCI interface local bus number.
*
* DESCRIPTION:
*       This function gets the local bus number of a given PCI interface.
*
* INPUT:
*       pciIf  - PCI interface number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Local bus number.0xffffffff on Error
*
*******************************************************************************/
MV_U32 mvPciLocalBusNumGet(MV_U32 pciIf)
{
	MV_U32 pciP2PConfig;

	/* Parameter checking   */
	if (PCI_DEFAULT_IF != pciIf)
	{
		if (pciIf >= mvCtrlPciMaxIfGet())
		{
			mvOsPrintf("mvPciLocalBusNumGet: ERR. Invalid PCI interface %d\n", 
													   					pciIf);
			return 0xFFFFFFFF;
		}
	}


	pciP2PConfig  = MV_REG_READ(PCI_P2P_CONFIG_REG(pciIf));

	pciP2PConfig &= PPCR_BUS_NUM_MASK;

	return (pciP2PConfig >> PPCR_BUS_NUM_OFFS);

}


/*******************************************************************************
* mvPciLocalDevNumSet - Set PCI interface local device number.
*
* DESCRIPTION:
*       This function sets given PCI interface its local device number.
*       Note: In case the PCI interface is PCI-X, the information is read-only.
*
* INPUT:
*       pciIf  - PCI interface number.
*       devNum - Device number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_NOT_ALLOWED in case PCI interface is PCI-X. MV_BAD_PARAM on bad parameters ,
*       otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciLocalDevNumSet(MV_U32 pciIf, MV_U32 devNum)
{
	MV_U32 pciP2PConfig;
	MV_PCI_MODE pciMode;
	MV_U32 localBus;
	MV_U32 localDev;

	/* Parameter checking   */
	if (pciIf >= mvCtrlPciMaxIfGet())
	{
		mvOsPrintf("mvPciLocalDevNumSet: ERR. Invalid PCI interface %d\n",pciIf);
		return MV_BAD_PARAM;
	}
	if (devNum >= MAX_PCI_DEVICES)
	{
		mvOsPrintf("mvPciLocalDevNumSet: ERR. device number illigal %d\n", 
																	   devNum);
		return MV_BAD_PARAM;

	}
	
	localBus = mvPciLocalBusNumGet(pciIf);
	localDev = mvPciLocalDevNumGet(pciIf);

	/* PCI interface mode */
	mvPciModeGet(pciIf, &pciMode);

	/* if PCI type is PCIX then it is not allowed to change the dev number */
	if (MV_PCIX == pciMode.pciType)
	{
		pciP2PConfig = mvPciConfigRead(pciIf, localBus, localDev, 0, PCIX_STATUS );

		pciP2PConfig &= ~PXS_DN_MASK;

		pciP2PConfig |= (devNum << PXS_DN_OFFS) & PXS_DN_MASK;

		mvPciConfigWrite(pciIf,localBus, localDev, 0, PCIX_STATUS,pciP2PConfig );
	}
	else
	{
		pciP2PConfig  = MV_REG_READ(PCI_P2P_CONFIG_REG(pciIf));

		pciP2PConfig &= ~PPCR_DEV_NUM_MASK;

		pciP2PConfig |= (devNum << PPCR_DEV_NUM_OFFS) & PPCR_DEV_NUM_MASK;

		MV_REG_WRITE(PCI_P2P_CONFIG_REG(pciIf), pciP2PConfig);

	}



	return MV_OK;
}

/*******************************************************************************
* mvPciLocalDevNumGet - Get PCI interface local device number.
*
* DESCRIPTION:
*       This function gets the local device number of a given PCI interface.
*
* INPUT:
*       pciIf  - PCI interface number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Local device number. 0xffffffff on Error
*
*******************************************************************************/
MV_U32 mvPciLocalDevNumGet(MV_U32 pciIf)
{
	MV_U32 pciP2PConfig;

	/* Parameter checking   */
	
	if (PCI_DEFAULT_IF != pciIf)
	{
		if (pciIf >= mvCtrlPciMaxIfGet())
		{
			mvOsPrintf("mvPciLocalDevNumGet: ERR. Invalid PCI interface %d\n", 
																   		pciIf);
			return 0xFFFFFFFF;
		}
	}
	
	pciP2PConfig  = MV_REG_READ(PCI_P2P_CONFIG_REG(pciIf));

	pciP2PConfig &= PPCR_DEV_NUM_MASK;

	return (pciP2PConfig >> PPCR_DEV_NUM_OFFS);
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

#if 0
/*******************************************************************************
* mvPciXbarCtrlSet - Set the arbiter`s priority for PCI interface.
*
* INPUT:
*       pPizzaArbArray - A priority Structure describing 16 "pizza slices". At 
*                    each clock cycle, the crossbar arbiter samples all 
*                    requests and gives the bus to the next agent according 
*                    to the "pizza".
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciXbarCtrlSet(MV_U32 pciIf, MV_XBAR_TARGET *pPizzaArbArray)
{
	
	MV_U32 sliceNum;
	MV_U32 xbarCtrl = 0;
	MV_XBAR_TARGET xbarTarget;

	/* 1) Set crossbar control low register */
	for (sliceNum = 0; sliceNum < PICCR_SLICE_NUM; sliceNum++)
	{
		xbarTarget = pPizzaArbArray[sliceNum];
		
		/* sliceNum parameter check */
		if ((xbarTarget > XBAR_MAX_TARGET) || 
			(xbarTarget == DEV_TARGET)     ||
			(xbarTarget == DRAM_TARGET))
		{
			mvOsPrintf("mvPciXbarCtrlSet: ERR. Can't set Target %d\n", 
																  xbarTarget);
			return MV_ERROR;
		}
		xbarCtrl |= (xbarTarget << PICCR_ARB_OFFS(sliceNum));
	}
	/* Write to crossbar control low register */
    MV_REG_WRITE(PCI_XBAR_CTRL_L_REG(pciIf), xbarCtrl);
				 
	xbarCtrl = 0;

	/* 2) Set crossbar control high register */
	for (sliceNum = 0; sliceNum < PICCR_SLICE_NUM; sliceNum++)
	{
		
		xbarTarget = pPizzaArbArray[sliceNum + PICCR_SLICE_NUM];
		
		/* sliceNum parameter check */
		if ((xbarTarget > XBAR_MAX_TARGET) || 
			(xbarTarget == DEV_TARGET)     ||
			(xbarTarget == DRAM_TARGET))
		{   
			mvOsPrintf("mvPciXbarCtrlSet: ERR. Can't set Target %d\n", 
																  xbarTarget);
			return MV_ERROR;
		}
		xbarCtrl |= (xbarTarget << PICCR_ARB_OFFS(sliceNum));
	}
	/* Write to crossbar control high register */
    MV_REG_WRITE(PCI_XBAR_CTRL_H_REG(pciIf), xbarCtrl);

	return MV_OK;
}

/*******************************************************************************
* mvPciXbarToutSet - Set PCI crossbar timeout value.
*
* DESCRIPTION:
*       This function sets PCI crossbar timeout value
*       according to input parameter. It will also enable timeout if enable 
*       parameter is MV_TRUE.
*
* INPUT:
*       enable  - Enable/disable parameter. If enable = MV_TRUE then enable.
*       timeout - crossbar timeout value. Maximum timeout value defined
*                 by PICTR_TIMEOUT_MAX.
*
* OUTPUT:
*       N/A
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPciXbarToutSet(MV_U32 pciIf, MV_BOOL enable, MV_U32 timeout)
{
	/* Parameter checking */
	if (timeout > PICTR_TIMEOUT_MAX)
	{
	
        mvOsPrintf("mvPciXbarToutSet: ERR. Timeout 0x%x invalid\n", timeout);
		return MV_ERROR;
	}

	if (enable == MV_FALSE)
	{
		/* Disable crossbar timeout              							*/
		MV_REG_WRITE(PCI_XBAR_TIMEOUT_REG(pciIf), PICTR_DISABLE);
	}
	else
	{
		/* Write timeout value and enable timeout 							*/
		MV_REG_WRITE(PCI_XBAR_TIMEOUT_REG(pciIf), 
											 (timeout << PICTR_TIMEOUT_OFFS));
	}

	return MV_OK;
}

#endif /* #if 0 */
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
		case CS0_BAR:      		/* SDRAM chip select 0 bar*/
			pBarRegInfo->funcNum          = 0;
			pBarRegInfo->baseLowRegOffs   = PCI_SCS0_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_SCS0_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs      = PCI_CS0_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_CS0_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		case CS1_BAR:      		/* SDRAM chip select 1 bar*/
			pBarRegInfo->funcNum          = 0;
			pBarRegInfo->baseLowRegOffs   = PCI_SCS1_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_SCS1_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs      = PCI_CS1_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_CS1_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		case MEM_INTER_REGS_BAR: /* Memory Mapped Internal bar */
			pBarRegInfo->funcNum          = 0;
			pBarRegInfo->baseLowRegOffs   = PCI_INTER_REG_MEM_MAPPED_BASE_ADDR_L;
			pBarRegInfo->baseHighRegOffs  = PCI_INTER_REG_MEM_MAPPED_BASE_ADDR_H;
			pBarRegInfo->sizeRegOffs      = 0;
			pBarRegInfo->remapLowRegOffs  = 0;
			pBarRegInfo->remapHighRegOffs = 0;
			break;
	
		/* Function 1 Bars */
		case CS2_BAR:      		/* SDRAM chip select 2 bar*/
			pBarRegInfo->funcNum          = 1;
			pBarRegInfo->baseLowRegOffs   = PCI_SCS2_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_SCS2_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs      = PCI_CS2_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_CS2_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		case CS3_BAR:      		/* SDRAM chip select 3 bar*/
			pBarRegInfo->funcNum		  = 1;
			pBarRegInfo->baseLowRegOffs	  = PCI_SCS3_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_SCS3_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_CS3_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_CS3_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
	
		/* Function 2 Bars */
		case DEVCS0_BAR:     	/* Device chip select 0 bar*/
			pBarRegInfo->funcNum		  = 2;
			pBarRegInfo->baseLowRegOffs	  = PCI_DEVCS0_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_DEVCS0_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_DEVCS0_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_DEVCS0_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		case DEVCS1_BAR:     	/* Device chip select 0 bar*/
			pBarRegInfo->funcNum		  = 2;
			pBarRegInfo->baseLowRegOffs	  = PCI_DEVCS1_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_DEVCS1_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_DEVCS1_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_DEVCS1_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
		case DEVCS2_BAR:     	/* Device chip select 0 bar*/
			pBarRegInfo->funcNum		  = 2;
			pBarRegInfo->baseLowRegOffs	  = PCI_DEVCS2_BASE_ADDR_LOW;
			pBarRegInfo->baseHighRegOffs  = PCI_DEVCS2_BASE_ADDR_HIGH;
			pBarRegInfo->sizeRegOffs	  = PCI_DEVCS2_BAR_SIZE_REG(pciIf);
			pBarRegInfo->remapLowRegOffs  = PCI_DEVCS2_ADDR_REMAP_REG(pciIf);
			pBarRegInfo->remapHighRegOffs = 0;
			break;
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
		case CS0_BAR: 
			return "CS0_BAR..............";
		case CS1_BAR: 
			return "CS1_BAR..............";
		case CS2_BAR: 
			return "CS2_BAR..............";
		case CS3_BAR: 
			return "CS3_BAR..............";
		case DEVCS0_BAR: 
			return "DEVCS0_BAR...........";
		case DEVCS1_BAR: 
			return "DEVCS1_BAR...........";
		case DEVCS2_BAR: 
			return "DEVCS2_BAR...........";
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

#elif defined(MV_88F1181)


/* PCI\PCIX stub function */
#include "mvPci.h"
#include "mvCpuIf.h"

MV_STATUS mvPciInit(MV_U32 pciIf)
{
	return MV_OK;
}
MV_U32 mvPciConfigRead (MV_U32 pciIf, MV_U32 bus, MV_U32 dev, MV_U32 func, 
                        MV_U32 regOff)
{
	return 0;
}

MV_STATUS mvPciConfigWrite(MV_U32 pciIf, MV_U32 bus, MV_U32 dev, 
                           MV_U32 func, MV_U32 regOff, MV_U32 data)
{
	return MV_OK;
}

MV_STATUS mvPciMasterEnable(MV_U32 pciIf, MV_BOOL enable)
{
	return MV_OK;
}

MV_STATUS mvPciSlaveEnable(MV_U32 pciIf, MV_U32 bus, MV_U32 dev, MV_BOOL enable)
{
	return MV_OK;
}
MV_STATUS mvPciLocalBusNumSet(MV_U32 pciIf, MV_U32 busNum)
{
	return MV_OK;
}
MV_U32 mvPciLocalBusNumGet(MV_U32 pciIf)
{
	return 0;
}
MV_STATUS mvPciLocalDevNumSet(MV_U32 pciIf, MV_U32 devNum)
{
	return MV_OK;
}
MV_U32 mvPciLocalDevNumGet(MV_U32 pciIf)
{
	return 0;
}

#else
#   error "CHIP not selected"
#endif /* #if defined(MV_88F5181) */



