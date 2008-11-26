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

#ifndef __NS16550_H__
#define __NS16550_H__

#include "mvCommon.h"
#include "mvOs.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"

#ifdef MV78XX0
#include "idma/mvIdma.h"
#include "ctrlEnv/sys/mvSysIdma.h"
#endif
/* This structure describes the registers offsets for one UART port(channel) */
typedef struct mvUartPort
{
	MV_U8 rbr;  /* 0 = 0-3*/
	MV_U8 pad1[3];

	MV_U8 ier;  /* 1 = 4-7*/
	MV_U8 pad2[3];

	MV_U8 fcr;  /* 2 = 8-b*/
	MV_U8 pad3[3];

	MV_U8 lcr;  /* 3 = c-f*/
	MV_U8 pad4[3];

	MV_U8 mcr;  /* 4 = 10-13*/
	MV_U8 pad5[3];

	MV_U8 lsr;  /* 5 = 14-17*/
	MV_U8 pad6[3];

	MV_U8 msr;  /* 6 =18-1b*/
	MV_U8 pad7[3];

	MV_U8 scr;  /* 7 =1c-1f*/
	MV_U8 pad8[3];
} MV_UART_PORT;

#if defined(MV_UART_OVER_PEX_WA) || defined(MV_UART_OVER_PCI_WA)
#define mvUartBase(port)  \
		((MV_UART_PORT *)(0xF2000000 + MV_UART_CHAN_BASE(port)))
#else
#define mvUartBase(port)  \
		((MV_UART_PORT *)(INTER_REGS_BASE + MV_UART_CHAN_BASE(port)))
#endif

/* aliases - for registers which has the same offsets */
#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

/* registers feilds */
#define FCR_FIFO_EN     	BIT0    /* fifo enable*/
#define FCR_RXSR        	BIT1    /* reciever soft reset*/
#define FCR_TXSR        	BIT2    /* transmitter soft reset*/

#define MCR_RTS         	BIT1	/* ready to send */
#define MCR_AFCE         	BIT5	/* Auto Flow Control Enable */

#define LCR_WLS_OFFS		0
#define LCR_WLS_MASK 		0x3 << LCR_WLS_OFFS    /* character length mask  */
#define LCR_WLS_5   		0x0 << LCR_WLS_OFFS    /* 5 bit character length */
#define LCR_WLS_6   		0x1 << LCR_WLS_OFFS    /* 6 bit character length */
#define LCR_WLS_7   		0x2 << LCR_WLS_OFFS    /* 7 bit character length */
#define LCR_WLS_8   		0x3 << LCR_WLS_OFFS    /* 8 bit character length */
#define LCR_STP_OFFS		2
#define LCR_1_STB     		0x0 << LCR_STP_OFFS   /* Number of stop Bits */
#define LCR_2_STB     		0x1 << LCR_STP_OFFS   /* Number of stop Bits */
#define LCR_PEN     		0x8    		      /* Parity eneble*/
#define LCR_PS_OFFS		4
#define LCR_EPS     		0x1 << LCR_PS_OFFS    /* Even Parity Select*/
#define LCR_OPS     		0x0 << LCR_PS_OFFS    /* Odd Parity Select*/
#define LCR_SBRK_OFFS		0x6
#define LCR_SBRK    		0x1 << LCR_SBRK_OFFS  /* Set Break*/
#define LCR_DIVL_OFFS		7
#define LCR_DIVL_EN    		0x1 << LCR_DIVL_OFFS   /* Divisior latch enable*/

#define LSR_DR      		BIT0    /* Data ready */
#define LSR_OE      		BIT1    /* Overrun */
#define LSR_PE      		BIT2    /* Parity error */
#define LSR_FE      		BIT3    /* Framing error */
#define LSR_BI      		BIT4    /* Break */
#define LSR_THRE    		BIT5    /* Xmit holding register empty */
#define LSR_TEMT    		BIT6    /* Xmitter empty */
#define LSR_ERR     		BIT7    /* Error */

/* useful defaults for LCR*/
#define LCR_8N1     LCR_WLS_8 | LCR_1_STB 


/* APIs */
MV_VOID	mvUartPutc(MV_U32 port, MV_U8 c);
MV_U8	mvUartGetc(MV_U32 port);
MV_BOOL mvUartTstc(MV_U32 port);
MV_VOID mvUartInit(MV_U32 port, MV_U32 baudDivisor, MV_UART_PORT* base);

#endif

