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
#if defined(MV_88F5182)

#include "mvXor.h"
#include "mvCpuIf.h"

/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	



static MV_STATUS xorWinOverlapDetect(MV_U32 winNum, MV_ADDR_WIN *pAddrWin);

MV_TARGET xorAddrDecPrioTap[] = 
{
    DEVICE_CS0,
	PEX0_MEM,
	PCI0_MEM,
	PCI0_IO,
	SDRAM_CS0,
	SDRAM_CS1,
	SDRAM_CS2,
	SDRAM_CS3,
   	DEVICE_CS1,
    TBL_TERM
};

/*******************************************************************************
* mvXorInit - Initialize XOR engine
*
* DESCRIPTION:
*		This function initialize XOR unit. It set the default address decode
*		windows of the unit.
*		Note that if the address window is disabled in xorAddrDecMap, the
*		window parameters will be set but the window will remain disabled.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*******************************************************************************/
MV_STATUS mvXorInit (MV_VOID)
{
	MV_U32         winNum;
	MV_U32         xorChanNum;
	MV_XOR_DEC_WIN addrDecWin;
	MV_CPU_DEC_WIN cpuAddrDecWin;
	MV_U32          status;
	MV_U32			winPrioIndex=0;

	/* Initiate XOR address decode */

	/* First disable all address decode windows */
	for(winNum = 0; winNum < XOR_MAX_ADDR_DEC_WIN; winNum++)
	{
		mvXorTargetWinEnable(winNum, MV_FALSE);
	}
	
	/* Go through all windows in user table until table terminator			*/
	for (winNum = 0; ((xorAddrDecPrioTap[winPrioIndex] != TBL_TERM) &&
					(winNum < XOR_MAX_ADDR_DEC_WIN));)
	{
		/* first get attributes from CPU If */
		status = mvCpuIfTargetWinGet(xorAddrDecPrioTap[winPrioIndex], 
									 &cpuAddrDecWin);

        if(MV_NO_SUCH == status)
        {
            winPrioIndex++;
            continue;
        }
		if (MV_OK != status)
		{
            mvOsPrintf("%s: ERR. mvCpuIfTargetWinGet failed\n", __FUNCTION__);
			return MV_ERROR;
		}

	
        if (cpuAddrDecWin.enable == MV_TRUE)
		{

			addrDecWin.target           = xorAddrDecPrioTap[winPrioIndex];
			addrDecWin.addrWin.baseLow  = cpuAddrDecWin.addrWin.baseLow; 
			addrDecWin.addrWin.baseHigh = cpuAddrDecWin.addrWin.baseHigh; 
			addrDecWin.addrWin.size     = cpuAddrDecWin.addrWin.size;    
			addrDecWin.enable           = MV_TRUE;
	
			if (MV_OK != mvXorTargetWinSet(winNum, &addrDecWin))
			{
				DB(mvOsPrintf("mvXorInit: ERR. mvDmaTargetWinSet failed\n"));
				return MV_ERROR;
			}
			winNum++;
		}
		winPrioIndex++;	
        
	}
	
	/* Abort any XOR activity */
	for(xorChanNum = 0; xorChanNum < MV_XOR_MAX_CHAN; xorChanNum++)
	{
		mvXorCommandSet(xorChanNum, MV_STOP);
	}
    
	return MV_OK;
}

/*******************************************************************************
* mvXorCtrlSet - Set XOR channel control registers
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
* NOTE:
*    This function does not modify the OperationMode field of control register.
*
*******************************************************************************/
MV_STATUS mvXorCtrlSet(MV_U32 chan, MV_U32 xorCtrl)
{
	MV_U32 oldValue;
    
	/* Parameter checking   */
	if (chan >= MV_XOR_MAX_CHAN)
	{
		DB(mvOsPrintf("mvXorCtrlSet: ERR. Invalid chan num %d\n", chan));
		return MV_BAD_PARAM;
	}

	/* update the XOR Engine [0..1] Configuration Registers (XExCR) */
	oldValue = MV_REG_READ(XOR_CONFIG_REG(chan)) & XEXCR_OPERATION_MODE_MASK;
	xorCtrl  &= ~XEXCR_OPERATION_MODE_MASK;
	xorCtrl  |= oldValue;
	MV_REG_WRITE(XOR_CONFIG_REG(chan),xorCtrl);
    return MV_OK;
}

/*******************************************************************************
* mvXorEccClean - .
*
* DESCRIPTION:
*       
*
* INPUT:
*       destPtr - 
*
* OUTPUT:
*       None.
*
* RETURN:        
*
*******************************************************************************/
MV_STATUS mvXorEccClean(MV_U32 chan, MV_XOR_ECC *pXorEccConfig)
{
    MV_U32  tClkCycles;
    MV_U32  temp;

    /* Parameter checking   */
    if (chan >= MV_XOR_MAX_CHAN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid chan num %d\n",__FUNCTION__ ,chan));
        return MV_BAD_PARAM;
    }
    if (NULL == pXorEccConfig)
    {
        DB(mvOsPrintf("%s: ERR. pXorEccConfig is NULL pointer\n",
                          __FUNCTION__ ));
        return MV_BAD_PTR;
    }
    if (MV_ACTIVE == mvXorStateGet(chan))
    {
        DB(mvOsPrintf("%s: ERR. Channel is already active\n", __FUNCTION__ ));
        return MV_BUSY;
    }
    if ((pXorEccConfig->sectorSize < XETMCR_SECTION_SIZE_MIN_VALUE) || 
        (pXorEccConfig->sectorSize > XETMCR_SECTION_SIZE_MAX_VALUE))
    {
        DB(mvOsPrintf("%s: ERR. sectorSize must be between %d to %d\n",
                       __FUNCTION__,XETMCR_SECTION_SIZE_MIN_VALUE,
                          XETMCR_SECTION_SIZE_MAX_VALUE));
        return MV_BAD_PARAM;
    }
    if ((pXorEccConfig->blockSize < XEXBSR_BLOCK_SIZE_MIN_VALUE) ||
        (pXorEccConfig->blockSize > XEXBSR_BLOCK_SIZE_MAX_VALUE))
    {
        DB(mvOsPrintf("%s: ERR. Block size must be between %d to %ul\n",
                       __FUNCTION__,XEXBSR_BLOCK_SIZE_MIN_VALUE,
                          XEXBSR_BLOCK_SIZE_MAX_VALUE));
        return MV_BAD_PARAM;
    }
    if (0x0 == pXorEccConfig->destPtr)
    {
        DB(mvOsPrintf("%s: ERR. destPtr is NULL pointer\n",__FUNCTION__ ));
        return MV_BAD_PARAM;
    }

    /* set the operation mode to ECC */
    temp = MV_REG_READ(XOR_CONFIG_REG(chan));
    temp &= ~XEXCR_OPERATION_MODE_MASK;
    temp |= XEXCR_OPERATION_MODE_ECC;
    MV_REG_WRITE(XOR_CONFIG_REG(chan), temp);
    
    /* update the TimerEn bit in the XOR Engine Timer Mode 
    Control Register (XETMCR) */
    if (pXorEccConfig->periodicEnable)
    {
        MV_REG_BIT_SET(XOR_TIMER_MODE_CTRL_REG,XETMCR_TIMER_EN_MASK);
    }
    else
    {
        MV_REG_BIT_RESET(XOR_TIMER_MODE_CTRL_REG,XETMCR_TIMER_EN_MASK);
    }

    /* update the SectionSizeCtrl bit in the XOR Engine Timer Mode Control 
    Register (XETMCR) */
    temp = MV_REG_READ(XOR_TIMER_MODE_CTRL_REG);
    temp &= ~XETMCR_SECTION_SIZE_CTRL_MASK;
    temp |= (pXorEccConfig->sectorSize << XETMCR_SECTION_SIZE_CTRL_OFFS);
    MV_REG_WRITE(XOR_TIMER_MODE_CTRL_REG, temp);

    /* update the DstPtr field in the XOR Engine [0..1] Destination Pointer 
    Register (XExDPR0) */
    MV_REG_WRITE(XOR_DST_PTR_REG(chan), pXorEccConfig->destPtr);

    /* update the BlockSize field in the XOR Engine[0..1] Block Size 
    Registers (XExBSR) */
    MV_REG_WRITE(XOR_BLOCK_SIZE_REG(chan), pXorEccConfig->blockSize);

    /* update the XOR Engine Timer Mode Initial Value Register (XETMIVR) */
    tClkCycles = pXorEccConfig->tClkTicks;
    MV_REG_WRITE(XOR_TIMER_MODE_INIT_VAL_REG, tClkCycles);
    
    /* start transfer */
    MV_REG_BIT_SET(XOR_ACTIVATION_REG(chan), XEXACTR_XESTART_MASK);
    
	return MV_OK;
}

/*******************************************************************************
* mvXorEccCurrTimerGet - Return ECC timer current value.
*
* DESCRIPTION:
*       Return the ECC timer mode Current value.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:        
*       Timer ticks (in Tclk frequancy).
*
*******************************************************************************/
MV_U32 mvXorEccCurrTimerGet(MV_U32 tClk)
{
    /* read the current Tclk */
    return (MV_REG_READ(XOR_TIMER_MODE_CURR_VAL_REG));
}

/*******************************************************************************
* mvXorMemInit - 
*
* DESCRIPTION:
*       
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:        
*       
*
*******************************************************************************/
MV_STATUS mvXorMemInit(MV_U32 chan, MV_U32 startPtr, MV_U32 blockSize, 
                       MV_U32 initValHigh, MV_U32 initValLow)
{
    MV_U32 temp;

    /* Parameter checking   */
    if (chan >= MV_XOR_MAX_CHAN)
    {
		mvOsPrintf("%s: ERR. Invalid chan num %d\n",__FUNCTION__ , chan);
        return MV_BAD_PARAM;
    }
    if (MV_ACTIVE == mvXorStateGet(chan))
    {
        mvOsPrintf("%s: ERR. Channel is already active\n", __FUNCTION__ );
        return MV_BUSY;
    }
    if ((blockSize < XEXBSR_BLOCK_SIZE_MIN_VALUE) ||
        (blockSize > XEXBSR_BLOCK_SIZE_MAX_VALUE))
    {
        mvOsPrintf("%s: ERR. Block size must be between %d to %ul\n",
                       __FUNCTION__,XEXBSR_BLOCK_SIZE_MIN_VALUE,
                          XEXBSR_BLOCK_SIZE_MAX_VALUE);
        return MV_BAD_PARAM;
    }
#if 0
/* tzachi - this is done purposely by u-boot */
    if (0x0 == startPtr)
    {
        mvOsPrintf("%s: ERR. startPtr is NULL pointer\n", __FUNCTION__ );
        return MV_BAD_PARAM;
    }
#endif

    /* set the operation mode to Memory Init */
    temp = MV_REG_READ(XOR_CONFIG_REG(chan));
    temp &= ~XEXCR_OPERATION_MODE_MASK;
    temp |= XEXCR_OPERATION_MODE_MEM_INIT;
    MV_REG_WRITE(XOR_CONFIG_REG(chan), temp);
    
    /* update the startPtr field in XOR Engine [0..1] Destination Pointer 
    Register (XExDPR0) */
    MV_REG_WRITE(XOR_DST_PTR_REG(chan), startPtr);

    /* update the BlockSize field in the XOR Engine[0..1] Block Size 
    Registers (XExBSR) */
    MV_REG_WRITE(XOR_BLOCK_SIZE_REG(chan), blockSize);

    /* update the field InitValL in the XOR Engine Initial Value Register 
    Low (XEIVRL) */
    MV_REG_WRITE(XOR_INIT_VAL_LOW_REG, initValLow);

    /* update the field InitValH in the XOR Engine Initial Value Register 
    High (XEIVRH) */
    MV_REG_WRITE(XOR_INIT_VAL_HIGH_REG, initValHigh);
    
    /* start transfer */
    MV_REG_BIT_SET(XOR_ACTIVATION_REG(chan), XEXACTR_XESTART_MASK);
    
	return MV_OK;
}

/*******************************************************************************
* mvXorTransfer - Transfer data from source to destination on one of 
*                 three modes (XOR,CRC32,DMA)
* 
* DESCRIPTION:       
*       This function initiates XOR channel, according to function parameters,
*       in order to perform XOR or CRC32 or DMA transaction.
*       To gain maximum performance the user is asked to keep the following 
*       restrictions:
*       1) Selected engine is available (not busy).
*       1) This module does not take into consideration CPU MMU issues.
*          In order for the XOR engine to access the appropreate source 
*          and destination, address parameters must be given in system 
*          physical mode.
*       2) This API does not take care of cache coherency issues. The source,
*          destination and in case of chain the descriptor list are assumed
*          to be cache coherent.
*       4) Parameters validity. For example, does size parameter exceeds 
*          maximum byte count of descriptor mode (16M or 64K).
*
* INPUT:
*       chan          - XOR channel number. See MV_XOR_CHANNEL enumerator.
*       xorType       - One of three: XOR, CRC32 and DMA operations.
*       xorChainPtr   - address of chain pointer
*
* OUTPUT:
*       None.
*
* RETURS:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*
*******************************************************************************/
MV_STATUS mvXorTransfer(MV_U32 chan, MV_XOR_TYPE xorType, MV_U32 xorChainPtr)
{
    MV_U32 temp;
    
    /* Parameter checking */
    if (chan >= MV_XOR_MAX_CHAN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid chan num %d\n",__FUNCTION__ , chan));
        return MV_BAD_PARAM;
    }
    if (MV_ACTIVE == mvXorStateGet(chan))
    {
        DB(mvOsPrintf("%s: ERR. Channel is already active\n", __FUNCTION__ ));
        return MV_BUSY;
    }
    if (0x0 == xorChainPtr)
    {
        DB(mvOsPrintf("%s: ERR. xorChainPtr is NULL pointer\n", __FUNCTION__ ));
        return MV_BAD_PARAM;
    }

    /* read configuration register and mask the operation mode field */
    temp = MV_REG_READ(XOR_CONFIG_REG(chan));
    temp &= ~XEXCR_OPERATION_MODE_MASK;

    switch (xorType)
    {
    case MV_XOR:
        if (0 != (xorChainPtr & XEXDPR_DST_PTR_XOR_MASK))
        {
            DB(mvOsPrintf("%s: ERR. Invalid chain pointer (bits [5:0] must "
                              "be cleared)\n",__FUNCTION__ ));
            return MV_BAD_PARAM;
        }
        /* set the operation mode to XOR */
        temp |= XEXCR_OPERATION_MODE_XOR;
        break;
        
    case MV_DMA:
        if (0 != (xorChainPtr & XEXDPR_DST_PTR_DMA_MASK))
        {
            DB(mvOsPrintf("%s: ERR. Invalid chain pointer (bits [4:0] must "
                              "be cleared)\n",__FUNCTION__ ));
            return MV_BAD_PARAM;
        }
        /* set the operation mode to DMA */
        temp |= XEXCR_OPERATION_MODE_DMA;
        break;
        
    case MV_CRC32:
        if (0 != (xorChainPtr & XEXDPR_DST_PTR_CRC_MASK))
        {
            DB(mvOsPrintf("%s: ERR. Invalid chain pointer (bits [4:0] must "
                              "be cleared)\n",__FUNCTION__ ));
            return MV_BAD_PARAM;
        }
        /* set the operation mode to CRC32 */
        temp |= XEXCR_OPERATION_MODE_CRC;
        break;
        
    default:
        return MV_BAD_PARAM;
    }
    
    /* write the operation mode to the register */
    MV_REG_WRITE(XOR_CONFIG_REG(chan), temp);
    /* update the NextDescPtr field in the XOR Engine [0..1] Next Descriptor 
    Pointer Register (XExNDPR) */
    MV_REG_WRITE(XOR_NEXT_DESC_PTR_REG(chan), xorChainPtr);
    
    /* start transfer */
    MV_REG_BIT_SET(XOR_ACTIVATION_REG(chan), XEXACTR_XESTART_MASK);
    
	return MV_OK;
}

/*******************************************************************************
* mvXorStateGet - Get XOR channel state.
*
* DESCRIPTION:
*       XOR channel activity state can be active, idle, paused.
*       This function retrunes the channel activity state.
*
* INPUT:
*       chan     - the channel number
*
* OUTPUT:
*       None.
*
* RETURN:
*       XOR_CHANNEL_IDLE    - If the engine is idle.
*       XOR_CHANNEL_ACTIVE  - If the engine is busy.
*       XOR_CHANNEL_PAUSED  - If the engine is paused.
*       MV_UNDEFINED_STATE  - If the engine state is undefind or there is no 
*                             such engine
*
*******************************************************************************/
MV_STATE mvXorStateGet(MV_U32 chan)
{
    MV_U32 state;
    
    /* Parameter checking   */
    if (chan >= MV_XOR_MAX_CHAN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid chan num %d\n",__FUNCTION__ , chan));
        return MV_UNDEFINED_STATE;
    }
    
    /* read the current state */
    state = MV_REG_READ(XOR_ACTIVATION_REG(chan));
    state &= XEXACTR_XESTATUS_MASK;
    
    /* return the state */
    switch (state)
    {
    case XEXACTR_XESTATUS_IDLE:
        return MV_IDLE;
    case XEXACTR_XESTATUS_ACTIVE:
        return MV_ACTIVE;
    case XEXACTR_XESTATUS_PAUSED:
        return MV_PAUSED;
    }
    return MV_UNDEFINED_STATE;
}

/*******************************************************************************
* mvXorCommandSet - Set command of XOR channel
*
* DESCRIPTION:
*       XOR channel can be started, idle, paused and restarted.
*       Paused can be set only if channel is active.
*       Start can be set only if channel is idle or paused.
*       Restart can be set only if channel is paused.
*       Stop can be set only if channel is active.    
*
* INPUT:
*       chan     - The channel number
*       command  - The command type (start, stop, restart, pause)    
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK on success , MV_BAD_PARAM on erroneous parameter, MV_ERROR on 
*       undefind XOR engine mode
*
*******************************************************************************/
MV_STATUS mvXorCommandSet(MV_U32 chan, MV_COMMAND command)
{
    MV_STATE    state;

    /* Parameter checking */
    if (chan >= MV_XOR_MAX_CHAN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid chan num %d\n",__FUNCTION__ , chan));
        return MV_BAD_PARAM;
    }

    /* get the current state */
    state = mvXorStateGet(chan);

    /* command is start and current state is idle */
    if ((command == MV_START) && (state == MV_IDLE))
    {
        MV_REG_BIT_SET(XOR_ACTIVATION_REG(chan), XEXACTR_XESTART_MASK);
        return MV_OK;
    }
    /* command is stop and current state is active*/
    else if ((command == MV_STOP) && (state == MV_ACTIVE))
    {
        MV_REG_BIT_SET(XOR_ACTIVATION_REG(chan), XEXACTR_XESTOP_MASK);
        return MV_OK;
    }
    /* command is paused and current state is active */
    else if ((command == MV_PAUSED) && (state == MV_ACTIVE))
    {
        MV_REG_BIT_SET(XOR_ACTIVATION_REG(chan), XEXACTR_XEPAUSE_MASK);
        return MV_OK;
    }
    /* command is restart and current state is paused*/
    else if ((command == MV_RESTART) && (state == MV_PAUSED))
    {
        MV_REG_BIT_SET(XOR_ACTIVATION_REG(chan), XEXACTR_XERESTART_MASK);
        return MV_OK;
    }
    /* command is stop and current state is active*/
    else if ((command == MV_STOP) && (state == MV_IDLE))
    {
        return MV_OK;
    }
    
    /* illegal command */
    DB(mvOsPrintf("%s: ERR. Illegal command\n", __FUNCTION__));
    
	return MV_BAD_PARAM;
}

/*******************************************************************************
* mvXorTargetWinSet - Set XOR target address window
*
* DESCRIPTION:
*       This function sets a peripheral target (e.g. SDRAM bank0, PCI_MEM0) 
*       address window. After setting this target window, the XOR will be 
*       able to access the target within the address window. 
*
* INPUT:
*	    winNum - One of the possible XOR memory decode windows.
*       target - Peripheral target enumerator.
*       base   - Window base address.
*       size   - Window size.
*       enable - Window enable/disable.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*
*******************************************************************************/
MV_STATUS mvXorTargetWinSet(MV_U32 winNum, MV_XOR_DEC_WIN *pAddrDecWin)
{
    MV_DEC_REGS xorDecRegs;
	MV_TARGET_ATTRIB targetAttribs;
    MV_U32      chan;
    
    /* Parameter checking */
    if (winNum >= XOR_MAX_ADDR_DEC_WIN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid win num %d\n",__FUNCTION__, winNum));
        return MV_BAD_PARAM;
    }
    if (pAddrDecWin == NULL)
    {
        DB(mvOsPrintf("%s: ERR. pAddrDecWin is NULL pointer\n", __FUNCTION__ ));
        return MV_BAD_PTR;
    }                                         
    /* Check if the requested window overlaps with current windows */
    if (MV_TRUE == xorWinOverlapDetect(winNum, &pAddrDecWin->addrWin))
	{
        DB(mvOsPrintf("%s: ERR. Window %d overlap\n",__FUNCTION__,winNum));
        return MV_ERROR;
	}                              

	xorDecRegs.baseReg = MV_REG_READ(XOR_BASE_ADDR_REG(winNum));
	xorDecRegs.sizeReg = MV_REG_READ(XOR_SIZE_MASK_REG(winNum));

    /* Get Base Address and size registers values */
    if(MV_OK != mvCtrlAddrDecToReg(&pAddrDecWin->addrWin, &xorDecRegs))
    {
		DB(mvOsPrintf("%s: ERR. Invalid addr dec window\n",__FUNCTION__));
        return MV_BAD_PARAM;
	}
    

	mvCtrlAttribGet(pAddrDecWin->target,&targetAttribs);

	/* set attributes */
	xorDecRegs.baseReg &= ~XEBARX_ATTR_MASK;
	xorDecRegs.baseReg |= targetAttribs.attrib << XEBARX_ATTR_OFFS;
	/* set target ID */
	xorDecRegs.baseReg &= ~XEBARX_TARGET_MASK;
	xorDecRegs.baseReg |= targetAttribs.targetId << XEBARX_TARGET_OFFS;


    /* Write to address decode Base Address Register */
    MV_REG_WRITE(XOR_BASE_ADDR_REG(winNum), xorDecRegs.baseReg);
    
    /* Write to Size Register */
    MV_REG_WRITE(XOR_SIZE_MASK_REG(winNum), xorDecRegs.sizeReg);
    
    for (chan = 0; chan < MV_XOR_MAX_CHAN; chan++)
    {
        if (pAddrDecWin->enable)
        {
            MV_REG_BIT_SET(XOR_WINDOW_CTRL_REG(chan),
                           XEXWCR_WIN_EN_MASK(winNum));
        }
        else
        {
            MV_REG_BIT_RESET(XOR_WINDOW_CTRL_REG(chan),
                             XEXWCR_WIN_EN_MASK(winNum));
        }
    }
    return MV_OK;
}

/*******************************************************************************
* mvXorTargetWinGet - Get xor peripheral target address window.
*
* DESCRIPTION:
*		Get xor peripheral target address window.
*
* INPUT:
*	  winNum - One of the possible XOR memory decode windows.
*
* OUTPUT:
*       base   - Window base address.
*       size   - Window size.
*       enable - window enable/disable.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*
*******************************************************************************/
MV_STATUS mvXorTargetWinGet(MV_U32 winNum, MV_XOR_DEC_WIN *pAddrDecWin)
{
    MV_DEC_REGS xorDecRegs;
	MV_TARGET_ATTRIB targetAttrib;
    MV_U32      chan=0,chanWinEn;
    
    /* Parameter checking */
    if (winNum >= XOR_MAX_ADDR_DEC_WIN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid win num %d\n",__FUNCTION__ , winNum));
        return MV_ERROR;
    }

    if (NULL == pAddrDecWin)
    {
        DB(mvOsPrintf("%s: ERR. pAddrDecWin is NULL pointer\n", __FUNCTION__ ));
        return MV_BAD_PTR;
    }

	chanWinEn = MV_REG_READ(XOR_WINDOW_CTRL_REG(0)) & XEXWCR_WIN_EN_MASK(winNum);
    
	for (chan = 0; chan < MV_XOR_MAX_CHAN; chan++)
    {
    	/* Check if enable bit is equal for all channels */
        if ((MV_REG_READ(XOR_WINDOW_CTRL_REG(chan)) & 
             XEXWCR_WIN_EN_MASK(winNum)) != chanWinEn)
        {
            mvOsPrintf("%s: ERR. Window enable field must be equal in "
                              "all channels\n",__FUNCTION__);
            return MV_ERROR;
        }
    }



	xorDecRegs.baseReg  = MV_REG_READ(XOR_BASE_ADDR_REG(winNum));
	xorDecRegs.sizeReg  = MV_REG_READ(XOR_SIZE_MASK_REG(winNum));

	if (MV_OK != mvCtrlRegToAddrDec(&xorDecRegs, &pAddrDecWin->addrWin))
	{
		mvOsPrintf("%s: ERR. mvCtrlRegToAddrDec failed\n", __FUNCTION__);
		return MV_ERROR;
	}

	/* attrib and targetId */
	targetAttrib.attrib = 
		(xorDecRegs.baseReg & XEBARX_ATTR_MASK) >> XEBARX_ATTR_OFFS;
	targetAttrib.targetId = 
		(xorDecRegs.baseReg & XEBARX_TARGET_MASK) >> XEBARX_TARGET_OFFS;


	pAddrDecWin->target = mvCtrlTargetGet(&targetAttrib);

	if(chanWinEn)
	{
		pAddrDecWin->enable = MV_TRUE;
	}
	else pAddrDecWin->enable = MV_FALSE;
	
    return MV_OK;
}

/*******************************************************************************
* mvXorTargetWinEnable - Enable/disable a Xor address decode window
*
* DESCRIPTION:
*       This function enable/disable a XOR address decode window.
*       if parameter 'enable' == MV_TRUE the routine will enable the 
*       window, thus enabling XOR accesses (before enabling the window it is 
*       tested for overlapping). Otherwise, the window will be disabled.
*
* INPUT:
*       winNum - Decode window number.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*
*******************************************************************************/
MV_STATUS mvXorTargetWinEnable(MV_U32 winNum, MV_BOOL enable)
{
	MV_XOR_DEC_WIN  addrDecWin;
    MV_U32          chan;
    
	/* Parameter checking   */               
    if (winNum >= XOR_MAX_ADDR_DEC_WIN)
    {
        DB(mvOsPrintf("%s: ERR. Invalid winNum%d\n", __FUNCTION__, winNum));
        return MV_ERROR;
    }

	if (enable == MV_TRUE) 
	{
		/* Get current window */
		if (MV_OK != mvXorTargetWinGet(winNum, &addrDecWin))
		{
			DB(mvOsPrintf("%s: ERR. targetWinGet fail\n", __FUNCTION__));
			return MV_ERROR;
		}

		/* Check for overlapping */
		if (MV_TRUE == xorWinOverlapDetect(winNum, &(addrDecWin.addrWin)))
		{
			/* Overlap detected	*/
			DB(mvOsPrintf("%s: ERR. Overlap detected\n", __FUNCTION__));
			return MV_ERROR;
		}

		/* No Overlap. Enable address decode target window */
		for (chan = 0; chan < MV_XOR_MAX_CHAN; chan++)
		{
			MV_REG_BIT_SET(XOR_WINDOW_CTRL_REG(chan),
						   XEXWCR_WIN_EN_MASK(winNum));
		}

	}
	else
	{
		/* Disable address decode target window */

		for (chan = 0; chan < MV_XOR_MAX_CHAN; chan++)
		{
			MV_REG_BIT_RESET(XOR_WINDOW_CTRL_REG(chan),
							 XEXWCR_WIN_EN_MASK(winNum));
		}

	}

	return MV_OK;                  
}

/*******************************************************************************
* mvXorSetProtWinSet - Configure access attributes of a XOR engine
*                               to one of the XOR memory windows.
*
* DESCRIPTION:
*       Each engine can be configured with access attributes for each of the
*       memory spaces. This function sets access attributes 
*       to a given window for the given engine
*
* INPUTS:
*       chan    - One of the possible engines.
*       winNum  - One of the possible XOR memory spaces.
*       access  - Protection access rights.
*       write   - Write rights.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*
*******************************************************************************/
MV_STATUS mvXorProtWinSet (MV_U32 chan, MV_U32 winNum, MV_BOOL access, 
                           MV_BOOL write)
{
    MV_U32 temp;
    
    /* Parameter checking   */
    if (chan >= MV_XOR_MAX_CHAN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid chan num %d\n", __FUNCTION__ , chan));
        return MV_BAD_PARAM;
    }
    if (winNum >= XOR_MAX_ADDR_DEC_WIN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid win num %d\n", __FUNCTION__, winNum));
        return MV_BAD_PARAM;
    }

    temp = MV_REG_READ(XOR_WINDOW_CTRL_REG(chan)) & 
        (~XEXWCR_WIN_ACC_MASK(winNum));

    /* if access is disable */
    if (!access)
    {
        /* disable access */
        temp |= XEXWCR_WIN_ACC_NO_ACC(winNum);
    }
    /* if access is enable */
    else
    {
        /* if write is enable */
        if (write)
        {
            /* enable write */
            temp |= XEXWCR_WIN_ACC_RW(winNum);
        }
        /* if write is disable */
        else
        {
            /* disable write */
            temp |= XEXWCR_WIN_ACC_RO(winNum);
        }
    }
    MV_REG_WRITE(XOR_WINDOW_CTRL_REG(chan),temp);
    return MV_OK;
}

/*******************************************************************************
* mvXorOverrideSet - Set XOR target window override
*
* DESCRIPTION:
*       The address override feature enables additional address decoupling. 
*       For example, it allows the use of the same source and destination 
*       addresses while the source is targeted to one interface and 
*       destination to a second interface.
*       XOR source/destination/next descriptor addresses can be override per
*       address decode windows 0,1,2 and 3 only. 
*       This function set override parameters per XOR channel. It access
*       XOR control register low.
*
* INPUT:
*       chan        - XOR channel number. See MV_XOR_CHANNEL enumerator.
*       winNum      - Override window number.
*                       Note: Not all windows can override.
*       override    - Type of override. See MV_XOR_OVERRIDE enumerator.
*       enable      - Window override is enabled or disabled
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*
*******************************************************************************/
MV_STATUS mvXorOverrideSet(MV_U32 chan, MV_XOR_OVERRIDE_TARGET target, 
                           MV_U32 winNum, MV_BOOL enable)
{
    MV_U32 temp;
    
    /* Parameter checking   */
    if (chan >= MV_XOR_MAX_CHAN)
    {
        
		DB(mvOsPrintf("%s: ERR. Invalid chan num %d\n", __FUNCTION__, chan));
        return MV_BAD_PARAM;
    }
    if (winNum >= XOR_MAX_OVERRIDE_WIN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid win num %d\n", __FUNCTION__, winNum));
        return MV_BAD_PARAM;
    }

    /* set the enable bit */
    if (enable)
    {
        MV_REG_BIT_SET(XOR_OVERRIDE_CTRL_REG(chan),XEXAOCR_OVR_EN_MASK(target));
    }
    else
    {
        MV_REG_BIT_RESET(XOR_OVERRIDE_CTRL_REG(chan),
                         XEXAOCR_OVR_EN_MASK(target));
    }

    /* read the override control register */
    temp = MV_REG_READ(XOR_OVERRIDE_CTRL_REG(chan));
    temp &= ~XEXAOCR_OVR_PTR_MASK(target);
    temp |= (winNum << XEXAOCR_OVR_PTR_OFFS(target));
    MV_REG_WRITE(XOR_OVERRIDE_CTRL_REG(chan), temp);
	return MV_OK;
}

/*******************************************************************************
* mvXorPciRemap - Set XOR remap register for PCI address windows.
*
* DESCRIPTION:
*       only Windows 0-3 can be remapped.
*
* INPUT:
*       winNum      - window number
*       pAddrDecWin  - pointer to address space window structure
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
*
*******************************************************************************/
MV_STATUS mvXorPciRemap(MV_U32 winNum, MV_U32 addrHigh)
{
    /* Parameter checking   */
    if (winNum >= XOR_MAX_REMAP_WIN)
    {
		DB(mvOsPrintf("%s: ERR. Invalid win num %d\n", __FUNCTION__, winNum));
        return MV_BAD_PARAM;
    }

    MV_REG_WRITE(XOR_HIGH_ADDR_REMAP_REG(winNum), addrHigh);
    
	return MV_OK;
}

/*******************************************************************************
* xorWinOverlapDetect - Detect XOR address windows overlaping
*
* DESCRIPTION:
*       An unpredicted behaviour is expected in case XOR address decode 
*       windows overlaps.
*       This function detects XOR address decode windows overlaping of a 
*       specified window. The function does not check the window itself for 
*       overlaping. The function also skipps disabled address decode windows.
*
* INPUT:
*       winNum      - address decode window number.
*       pAddrDecWin - An address decode window struct.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the given address window overlap current address
*       decode map, MV_FALSE otherwise, MV_ERROR if reading invalid data 
*       from registers.
*
*******************************************************************************/
static MV_STATUS xorWinOverlapDetect(MV_U32 winNum, MV_ADDR_WIN *pAddrWin)
{
	MV_U32 	        baseAddrEnableReg;
	MV_U32          winNumIndex,chan;
	MV_XOR_DEC_WIN  addrDecWin;

	if (pAddrWin == NULL)
	{
		DB(mvOsPrintf("%s: ERR. pAddrWin is NULL pointer\n", __FUNCTION__ ));
		return MV_BAD_PTR;
	}
    
	for (chan = 0; chan < MV_XOR_MAX_CHAN; chan++)
	{
		/* Read base address enable register. Do not check disabled windows	*/
		baseAddrEnableReg = MV_REG_READ(XOR_WINDOW_CTRL_REG(chan));

		for (winNumIndex = 0; winNumIndex < XOR_MAX_ADDR_DEC_WIN; winNumIndex++)
		{
			/* Do not check window itself */
			if (winNumIndex == winNum)
			{
				continue;
			}
    
			/* Do not check disabled windows */
			if ((baseAddrEnableReg & XEXWCR_WIN_EN_MASK(winNumIndex)) == 0)
			{
				continue;
			}
    
			/* Get window parameters */
			if (MV_OK != mvXorTargetWinGet(winNumIndex, &addrDecWin))
			{
				DB(mvOsPrintf("%s: ERR. TargetWinGet failed\n", __FUNCTION__ ));
				return MV_ERROR;
			}
            
			if (MV_TRUE == ctrlWinOverlapTest(pAddrWin, &(addrDecWin.addrWin)))
			{
				return MV_TRUE;
			}
		}
	}
	
	return MV_FALSE;
}


/*******************************************************************************
* mvXorAddrDecShow - Print the XOR address decode map.
*
* DESCRIPTION:
*		This function print the XOR address decode map.
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
MV_VOID mvXorAddrDecShow(MV_VOID)
{
	MV_XOR_DEC_WIN win;
	int            i;

	mvOsOutput( "\n" );
	mvOsOutput( "XOR:\n" );
	mvOsOutput( "----\n" );

	for( i = 0; i < XOR_MAX_ADDR_DEC_WIN; i++ )
	{
		memset( &win, 0, sizeof(MV_XOR_DEC_WIN) );

		mvOsOutput( "win%d - ", i );

		if( mvXorTargetWinGet( i, &win ) == MV_OK )
		{
			if( win.enable )
			{
				mvOsOutput( "%s base %x, ",
				mvCtrlTargetNameGet(win.target), win.addrWin.baseLow );

				mvSizePrint( win.addrWin.size );
				
                mvOsOutput( "\n" );
			}
			else
				mvOsOutput( "disable\n" );
		}
	}
}
#endif /* #if defined(MV_88F5182) */
