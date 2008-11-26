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



#include "mvUart.h"


/* static variables */
static volatile MV_UART_PORT* uartBase[MV_UART_MAX_CHAN]; 


/*******************************************************************************
* mvUartInit - Init a uart port.
*
* DESCRIPTION:
*       This routine Initialize one of the uarts ports (channels).
*	It initialize the baudrate, stop bit,parity bit etc.
*
* INPUT:
*       port - uart port number.
*	baudDivisor - baud divisior to use for the uart port.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvUartInit(MV_U32 port, MV_U32 baudDivisor, MV_UART_PORT* base)
{
	volatile MV_UART_PORT *pUartPort;

#if defined(MV_UART_OVER_PEX_WA) || defined(MV_UART_OVER_PCI_WA)
	uartBase[port] = pUartPort = (volatile MV_UART_PORT *)(base);
	return;
#else
	uartBase[port] = pUartPort = (volatile MV_UART_PORT *)base;

	pUartPort->ier = 0x00;
	pUartPort->lcr = LCR_DIVL_EN;           /* Access baud rate */
	pUartPort->dll = baudDivisor & 0xff;    /* 9600 baud */
	pUartPort->dlm = (baudDivisor >> 8) & 0xff;
	pUartPort->lcr = LCR_8N1;               /* 8 data, 1 stop, no parity */

	/* Clear & enable FIFOs */
	pUartPort->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;
	return;
#endif
}

#ifdef MV78XX0
#define UART_EXTERNAL_CONTROL_REG			0x10700
/*******************************************************************************
* mvUartDmaInit - Init a uart port in DMA mode.
*
* DESCRIPTION:
*       This routine Initialize one of the uart ports (channels) to use DMA mode
*	    for transmission. It initialize the baudrate, stop bit,parity bit etc.
*       It also initializes IDMA channel 0 for UART transmission.
*       Once DMA based UART is configured IDMA channel 0 must not be used elsewhere.
*
* INPUT:
*       port - uart port number.
*	    baudDivisor - baud divisior to use for the uart port.
*       idmaWinNum - The IDMA BAR to use for UART transfer (must be 1-3)
*       srcBurstSize - the IDMA source burst size (8,16,32,64,128)
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvUartDmaInit(MV_U32 port, MV_U32 baudDivisor, MV_UART_PORT* base,
                      MV_U32 idmaWinNum, MV_U32 srcBurstSize)
{
	volatile MV_UART_PORT *pUartPort;
    MV_U32  srcBurstLim;

    /*set the UART channel to use the IDMA*/
    MV_REG_BIT_SET(UART_EXTERNAL_CONTROL_REG, BIT0 | (BIT8 << port));

	uartBase[port] = pUartPort = (volatile MV_UART_PORT *)base;
    mvUartInit(port, baudDivisor, base);

    pUartPort->mcr = MCR_AFCE;           /* Enable Auto flow control */
    /* Set IDMA BAR target to UART*/
    MV_REG_BIT_RESET(IDMA_BASE_ADDR_REG(idmaWinNum), 0xffff);
    MV_REG_BIT_SET(IDMA_BASE_ADDR_REG(idmaWinNum), 0x0101); /*Set BAR to UART*/

    /* Set IDMA0 channel to UART mode*/
    switch (srcBurstSize)
    {
    case 8:
        srcBurstLim = ICCLR_SRC_BURST_LIM_8BYTE;
        break;
    case 16:
        srcBurstLim = ICCLR_SRC_BURST_LIM_16BYTE;
        break;
    case 32:
        srcBurstLim = ICCLR_SRC_BURST_LIM_32BYTE;
        break;
    case 64:
        srcBurstLim = ICCLR_SRC_BURST_LIM_64BYTE;
        break;
    case 128:
        srcBurstLim = ICCLR_SRC_BURST_LIM_128BYTE;
        break;
    default:
        mvOsPrintf("mvUartDmaInit:ERR. Illegal source burst limit, \
                        setting to 128B default\n");
        srcBurstLim = ICCLR_SRC_BURST_LIM_128BYTE;
        break;
    }
    mvDmaCtrlLowSet(0, 0x18220 | srcBurstLim); /*channel control low according FS*/
    mvDmaOverrideSet(0, idmaWinNum, DMA_DST_ADDR);
    return;
}


/*******************************************************************************
* mvUartDmaTransmit - Transmit a buffer over the UART using the IDMA.
*
* DESCRIPTION:
*       This routine transmits a buffer over the UART port using IDMA0.
*
* INPUT:
*       port - uart port number.
*	    byteCount - The total number of bytes to be transferred to the UART.
*       srcAddr - The source address of the buffer to be transferred.
*
* OUTPUT:
*       None.
*
* RETURN:
*	    MV_OK.
*
*******************************************************************************/
MV_STATUS mvUartDmaTransmit(MV_U32 port, MV_U32 byteCount, MV_U32 srcAddr)
{
    MV_U32 dstAddr, tmp;

    /* Verify IDMA 0 is not busy */
    if(mvDmaStateGet(port) != MV_IDLE)
    {
		mvOsPrintf("mvUartDmaTransmit: ERR. IDMA 0 busy.\n");
        return MV_ERROR;
    }

    /* Verify byte count is legal, and choose either 64K mode or 16M mode */
    if(byteCount >= 0x1000000)
    {
		mvOsPrintf("mvUartDmaTransmit: ERR. Illegal Byte count.\n");
        return MV_ERROR;
    }
    else if (byteCount >= 0x10000)
    {
        byteCount |= BIT31;
        MV_REG_BIT_SET(IDMA_CTRL_LOW_REG(0), BIT31);
    }
    else
        MV_REG_BIT_RESET(IDMA_CTRL_LOW_REG(0), BIT31);

    /* Calculate UART destination address from assigned BAR*/
    tmp = MV_REG_READ(IDMA_CTRL_LOW_REG(0));
    tmp = (tmp >> 23) & 0x3; /* Extract the destination override window number*/

    /* Check that destination BAR is configured and calculate destination address*/
    if((MV_REG_READ(IDMA_BASE_ADDR_REG(tmp)) & 0xffff) != 0x0101)
    {
		mvOsPrintf("mvUartDmaTransmit: ERR. IDMA UART BAR isn't configured.\n");
        return MV_ERROR;
    }
    else
        dstAddr = (MV_REG_READ(IDMA_BASE_ADDR_REG(tmp)) & 0xffff0000) | (port << 8);

    /* Start transfer */
    if(mvDmaTransfer(0, srcAddr, dstAddr, byteCount, 0x0) == MV_OK)
        return MV_OK;
    else
        return MV_ERROR;    
}
#endif // MV78XX0
/*******************************************************************************
* mvUartPutc - Send char to the uart port.
*
* DESCRIPTION:
*       This routine puts one charachetr on one of the uart ports.
*
* INPUT:
*       port - uart port number.
*	c - character.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID	mvUartPutc(MV_U32 port, MV_U8 c)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	while ((pUartPort->lsr & LSR_THRE) == 0) ;
	pUartPort->thr = c;
	return;
}

/*******************************************************************************
* mvUartGetc - Get char from uart port.
*
* DESCRIPTION:
*       This routine gets one charachetr from one of the uart ports.
*
* INPUT:
*       port - uart port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*	carachter from the uart port.
*
*******************************************************************************/
MV_U8	mvUartGetc(MV_U32 port)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	while ((pUartPort->lsr & LSR_DR) == 0) ;
	return (pUartPort->rbr);
}

/*******************************************************************************
* mvUartTstc - test for char in uart port.
*
* DESCRIPTION:
*       This routine heck if a charachter is ready to be read from one of the
*	the uart ports.
*
* INPUT:
*       port - uart port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_BOOL mvUartTstc(MV_U32 port)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	return ((pUartPort->lsr & LSR_DR) != 0);
}
