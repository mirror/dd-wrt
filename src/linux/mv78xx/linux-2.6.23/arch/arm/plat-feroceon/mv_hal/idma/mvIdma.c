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
* mvIdma.c - Implementation file for IDMA HW library
*
* DESCRIPTION:
*       This file contains Marvell Controller IDMA HW library API 
*       implementation.
*       NOTE: 
*       1) This HW library API assumes IDMA source, destination and 
*          descriptors are cache coherent. 
*       2) In order to gain high performance, the API does not parform 
*          API parameter checking.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#include "idma/mvIdma.h"

/* defines  */
#ifdef MV_DEBUG
        #define DB(x)   x
#else
        #define DB(x)
#endif  

/*******************************************************************************
* mvDmaInit - Initialize IDMA engine
*
* DESCRIPTION:
*               This function initialize IDMA unit. 
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_ERROR if setting fail.
*******************************************************************************/
MV_VOID mvDmaHalInit(MV_U32 dmaChanNum)
{
	MV_U32	i;

        /* Abort any DMA activity */
        for(i = 0; i < dmaChanNum; i++)
        {
                mvDmaCommandSet(i, MV_STOP);

                /* The following must be set */
                mvDmaCtrlHighSet(i, (ICCHR_ENDIAN_LITTLE  
#if defined(MV_CPU_LE)
				      | ICCHR_DESC_BYTE_SWAP_EN
#endif
				     ));

        }
    	MV_REG_WRITE( IDMA_CAUSE_REG, 0);
}

/*******************************************************************************
* mvDmaCtrlLowSet - Set IDMA channel control low register
*
* DESCRIPTION:
*       Each IDMA Channel has its own unique control registers (high and low)
*       where certain IDMA modes are programmed.
*       This function writes 32bit word to IDMA control low register.
*
*						!!!!!! WARNING !!!!!!
*		If system uses the IDMA DRAM HW cache coherency the source and 
* 		destination maximum DTL size must be cache line size.
*
* INPUT:
*       chan     - DMA channel number. See MV_DMA_CHANNEL enumerator.
*       ctrlWord - Channel control word for low register.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK
*
* NOTE: This function can modified the Override attribute that mvDmaOverrideSet
*       configured. 
*******************************************************************************/
MV_STATUS mvDmaCtrlLowSet(MV_U32 chan, MV_U32 ctrlWord)
{
	MV_REG_WRITE(IDMA_CTRL_LOW_REG(chan), ctrlWord);
	return MV_OK;
}


/*******************************************************************************
* mvDmaCtrlHighSet - Set IDMA channel control high register
*
* DESCRIPTION:
*       Each IDMA Channel has its own unique control registers (high and low)
*       where certain IDMA modes are programmed.
*       This function writes 32bit word to IDMA control high register.
*
* INPUT:
*       chan     - DMA channel number. See MV_DMA_CHANNEL enumerator.
*       ctrlWord - Channel control word for high register.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK.
*
*******************************************************************************/
MV_STATUS mvDmaCtrlHighSet(MV_U32 chan, MV_U32 ctrlWord)
{
    MV_REG_WRITE(IDMA_CTRL_HIGH_REG(chan), ctrlWord);
	return MV_OK;
}

/*******************************************************************************
* mvDmaTransfer - Transfer data from source to destination
* 
* DESCRIPTION:       
*       This function initiates IDMA channel, according to function parameters,
*       in order to perform DMA transaction.
*       This routine supports both chain and none chained DMA modes. 
*       To use the function in chain mode just set phyNextDesc parameter with
*       chain second descriptor address (the first one is given in other 
*       function paarameters). Otherwise (none chain mode) set it to NULL.
*       To gain maximum performance the user is asked to keep the following 
*       restrictions:
*       1) Selected engine is available (not busy).
*       1) This module does not take into consideration CPU MMU issues.
*          In order for the IDMA engine to access the appropreate source 
*          and destination, address parameters must be given in system 
*          physical mode.
*       2) This API does not take care of cache coherency issues. The source,
*          destination and in case of chain the descriptor list are assumed
*          to be cache coherent.
*       3) In case of chain mode, the API does not align the user descriptor
*          chain. Instead, the user must make sure the descriptor chain is 
*          aligned according to IDMA rquirements.
*       4) Parameters validity. For example, does size parameter exceeds 
*          maximum byte count of descriptor mode (16M or 64K).
*
* INPUT:
*       chan          - DMA channel number. See MV_DMA_CHANNEL enumerator.
*       phySrc        - Physical source address.
*       phyDst        - Physical destination address.
*       size          - The total number of bytes to transfer.
*       *pPhyNextDesc - Physical address pointer to 2nd descriptor in chain. 
*                       In case of none chain mode set it to NULL.
*
* OUTPUT:
*       None.
*
* RETURS:
*       MV_OK.
*
*******************************************************************************/
MV_STATUS mvDmaTransfer(MV_U32 chan, MV_U32 phySrc, MV_U32 phyDst, MV_U32 size, 
                                                    MV_U32 phyNextDescPtr)
{   
	/* Set byte count register			*/
	MV_REG_WRITE(IDMA_BYTE_COUNT_REG(chan), size);
	/* Set source address register		*/
	MV_REG_WRITE(IDMA_SRC_ADDR_REG(chan), phySrc);
	/* Set destination address register	*/
	MV_REG_WRITE(IDMA_DST_ADDR_REG(chan), phyDst);
	/* UnLock the Source address in dma operation */
	MV_REG_BIT_RESET(IDMA_CTRL_LOW_REG(chan), ICCLR_SRC_HOLD);
	
    if (0 != phyNextDescPtr)
	{	/* Chain mode. Set next descriptor register	*/
		MV_REG_WRITE(IDMA_NEXT_DESC_PTR_REG(chan), phyNextDescPtr);
	}
	
	/* Start DMA	*/
	MV_REG_BIT_SET(IDMA_CTRL_LOW_REG(chan), ICCLR_CHAN_ENABLE);
	
    return MV_OK;
}

/*******************************************************************************
* mvDmaMemInit - Initialize a memory buffer with a given 64bit value pattern
* 
* DESCRIPTION:       
*       This function initiates IDMA channel, according to function parameters,
*       in order to perform DMA transaction for the purpose of initializing a 
*       memory buffer with a user supplied pattern.
*       This routine supports both chain and none chained DMA modes. 
*       To use the function in chain mode just set phyNextDesc parameter with
*       chain second descriptor address (the first one is given in other 
*       function paarameters). Otherwise (none chain mode) set it to NULL.
*       To gain maximum performance the user is asked to keep the following 
*       restrictions:
*       1) Selected engine is available (not busy).
*       1) This module does not take into consideration CPU MMU issues.
*          In order for the IDMA engine to access the appropreate source 
*          and destination, address parameters must be given in system 
*          physical mode.
*       2) This API does not take care of cache coherency issues. The source,
*          destination and in case of chain the descriptor list are assumed
*          to be cache coherent.
*       3) No chain mode support.
*       4) Parameters validity. For example, does size parameter exceeds 
*          maximum byte count of descriptor mode (16M or 64K).
*
* INPUT:
*       chan          - DMA channel number. See MV_DMA_CHANNEL enumerator.
*       ptrnPtr       - Physical source address of the 64bit pattern
*       startPtr      - Physical destinaation address to start with
*       size          - The total number of bytes to transfer.
*
* OUTPUT:
*       None.
*
* RETURS:
*       MV_OK.
*
*******************************************************************************/
MV_STATUS mvDmaMemInit(MV_U32 chan, MV_U32 ptrnPtr, MV_U32 startPtr, MV_U32 size)
{   
	/* Set byte count register			*/
	MV_REG_WRITE(IDMA_BYTE_COUNT_REG(chan), size);
	/* Set source address register		*/
	MV_REG_WRITE(IDMA_SRC_ADDR_REG(chan), ptrnPtr);
	/* Set destination address register	*/
	MV_REG_WRITE(IDMA_DST_ADDR_REG(chan), startPtr);
	/* Lock the Source address in dma operation */
	MV_REG_BIT_SET(IDMA_CTRL_LOW_REG(chan), ICCLR_SRC_HOLD);
	
	/* Start DMA	*/
	MV_REG_BIT_SET(IDMA_CTRL_LOW_REG(chan), ICCLR_CHAN_ENABLE);
	
    return MV_OK;
}

/*******************************************************************************
* mvDmaStateGet - Get IDMA channel status.
*
* DESCRIPTION:
*       DMA channel status can be active, stopped, paused.
*       This function retrunes the channel status.
*
* INPUT:
*       chan - IDMA channel number. See MV_DMA_CHANNEL enumerator.
*
* OUTPUT:
*       None.
*
* RETURN:
*       One of MV_STATE enumerator values.
*
*******************************************************************************/
MV_STATE mvDmaStateGet(MV_U32 chan)
{
    MV_U32  ctrlLow;

    /* Read control low register    */
    ctrlLow = MV_REG_READ(IDMA_CTRL_LOW_REG(chan));
    
    /* Leave only enable/active bits    */
    ctrlLow &= (ICCLR_CHAN_ENABLE | ICCLR_CHAN_ACTIVE);

    /* If channel is enabled and active then its running        */
    if (ctrlLow == (ICCLR_CHAN_ENABLE | ICCLR_CHAN_ACTIVE))
    {
        return MV_ACTIVE;
    }
    /* If channel is disabled but active then its paused        */
    else if (ctrlLow == ICCLR_CHAN_ACTIVE)
    {
        return MV_PAUSED;
    }
    else 
    {    
        return MV_IDLE;
    }
}

/*******************************************************************************
* mvDmaCommandSet - Set command to DMA channel
*
* DESCRIPTION:
*       DMA channel can be started, idel, paused and restarted.
*       Paused can be set only if channel is active.
*       Start can be set only if channel is idle.
*       Restart can be set only if channel is paused.
*       Stop activate the channel abort which cause the DMA to aborts in the 
*       middle of a transaction.
*
* INPUT:
*       chan    - IDMA channel number. See MV_DMA_CHANNEL enumerator.
*       command - Requested command. See MV_COMMAND enumerator.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_ERROR if requested command can not be set.
*
*******************************************************************************/
MV_STATUS mvDmaCommandSet(MV_U32 chan, MV_COMMAND command)
{
    MV_U32  ctrlLow;
    MV_U32  dmaStatus;

    /* Read control low register    */
    ctrlLow = MV_REG_READ(IDMA_CTRL_LOW_REG(chan));

    /* Get current DMA status       */
    dmaStatus = mvDmaStateGet(chan);

    if ((command == MV_START) && (dmaStatus == MV_IDLE)) 
    {
        /* To start, set DMA channel enable bit                             */
        ctrlLow |= ICCLR_CHAN_ENABLE;
    }
    else if ((command == MV_PAUSE) && (dmaStatus == MV_ACTIVE)) 
    {
        /* To pause, reset DMA channel enable bit                           */
        ctrlLow &= ~ICCLR_CHAN_ENABLE;
    }
    else if ((command == MV_RESTART) && (dmaStatus == MV_PAUSED)) 
    {
        /* To restart, set DMA channel enable bit                           */
        ctrlLow |= ICCLR_CHAN_ENABLE;
    }
    else if (command == MV_STOP)
    {
        /* To stop, set DMA channel abort bit                               */
        ctrlLow |= ICCLR_CHANNEL_ABORT;
    }    
    else
    {
		DB(mvOsPrintf("mvDmaCommandSet: ERR. Can not set command %d in \
					   status %d\n", command, dmaStatus));
        return MV_ERROR;
    }
    
    /* Write control word to register   */
    MV_REG_WRITE(IDMA_CTRL_LOW_REG(chan), ctrlLow);

    /* If comman is stop, ensure channel is stopped                         */
    if (command == MV_STOP)
    {
        while(MV_IDLE != mvDmaStateGet(chan));
    }    

    return  MV_OK;
}


/************ DEBUG ***********/
MV_VOID    mvIdmaRegs(MV_U32 chan)
{
    mvOsPrintf("\t IDMA #%d Registers:\n", chan);

    mvOsPrintf("IDMA_BYTE_COUNT_REG             : 0x%X = 0x%08x\n", 
                IDMA_BYTE_COUNT_REG(chan), 
                MV_REG_READ( IDMA_BYTE_COUNT_REG(chan) ) );    

    mvOsPrintf("IDMA_SRC_ADDR_REG               : 0x%X = 0x%08x\n", 
                IDMA_SRC_ADDR_REG(chan), 
                MV_REG_READ( IDMA_SRC_ADDR_REG(chan) ) );    

    mvOsPrintf("IDMA_DST_ADDR_REG               : 0x%X = 0x%08x\n", 
                IDMA_DST_ADDR_REG(chan), 
                MV_REG_READ( IDMA_DST_ADDR_REG(chan) ) );    

    mvOsPrintf("IDMA_NEXT_DESC_PTR_REG          : 0x%X = 0x%08x\n", 
                IDMA_NEXT_DESC_PTR_REG(chan), 
                MV_REG_READ( IDMA_NEXT_DESC_PTR_REG(chan) ) );    

    mvOsPrintf("IDMA_CURR_DESC_PTR_REG          : 0x%X = 0x%08x\n", 
                IDMA_CURR_DESC_PTR_REG(chan), 
                MV_REG_READ( IDMA_CURR_DESC_PTR_REG(chan) ) );    

    mvOsPrintf("IDMA_CTRL_LOW_REG               : 0x%X = 0x%08x\n", 
                IDMA_CTRL_LOW_REG(chan), 
                MV_REG_READ( IDMA_CTRL_LOW_REG(chan) ) );    

    mvOsPrintf("IDMA_CTRL_HIGH_REG              : 0x%X = 0x%08x\n", 
                IDMA_CTRL_HIGH_REG(chan), 
                MV_REG_READ( IDMA_CTRL_HIGH_REG(chan) ) );    

    mvOsPrintf("IDMA_CAUSE_REG                  : 0x%X = 0x%08x\n", 
                IDMA_CAUSE_REG, 
                MV_REG_READ( IDMA_CAUSE_REG ) );    

    mvOsPrintf("IDMA_MASK_REG                   : 0x%X = 0x%08x\n", 
                IDMA_MASK_REG, 
                MV_REG_READ( IDMA_MASK_REG ) );    
}

