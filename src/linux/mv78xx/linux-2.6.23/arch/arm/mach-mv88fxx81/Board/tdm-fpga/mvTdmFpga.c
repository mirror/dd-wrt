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

#include "mvTdmFpga.h"
#include "mvTdmFpgaRegs.h"
#include "proslic.h"
#include "mvOs.h"


/* Access to FPGA registers (16bit device) */
#define REG16(x)	(*(volatile unsigned short *)(x))

#define CONTROL_REG_MASK (PROSLIC_RESET_CONTROL | RXDBLOCK_INT_MASK | SLIC_INT_MASK)

#define BUFF_IS_FULL	1
#define BUFF_IS_EMPTY	0

#define EXCEPTION_ON	1
#define EXCEPTION_OFF	0

#define MVDEBUG(fmt,args...) 
//mvOsPrintf

/* Low level loopback inside interrupt handler to check self echo on local phone */
/*#define IRQ_LOOPBACK*/
#ifdef IRQ_LOOPBACK
MV_VOID doIrqLoopback(MV_U16 cause)
{
	MV_U16 buff[MV_TDM_BUFF_SIZE/2];
	int i;

	/* opposite bits - sync ok */
	if( (cause & TXD_SLIC_REGFLAG) ^ (cause & TXD_ARM_REGFLAG) ) 
	{
		/* read from device */
		for(i=0; i<MV_TDM_BUFF_SIZE/2; i++)
			buff[i] = REG16(FPGA_DTX);
	}
	else {
		MVDEBUG("LOOP READ ERROR\n");
	}

	if( (cause & RXD_SLIC_REGFLAG) ^ (cause & RXD_ARM_REGFLAG) ) 
	{
		/* write to device */
		for(i=0; i<MV_TDM_BUFF_SIZE/2; i++)
			REG16(FPGA_DRX) = buff[i];
	}
	else 
	{
		MVDEBUG("LOOP WRITE ERROR\n");
	}
}
#endif

typedef struct _mv_slic_info
{
	MV_U8 family;
	MV_U8 name[32];
} MV_SLIC_INFO;

typedef struct _mv_tdm_ch_info
{
	MV_U8 ch;
	MV_U8 cs;
	MV_U8 active;
	MV_U8 *rxBuff, *txBuff;
	MV_U8 rxBuffFull, txBuffFull;
	MV_U8 exception;
	MV_SLIC_INFO slic;
} MV_TDM_CH_INFO;

MV_TDM_CH_INFO *tdmChInfo = NULL;

MV_STATUS mvTdmSpiRead(MV_U8 addr, MV_U8 *data);
MV_STATUS mvTdmSpiWrite(MV_U8 addr, MV_U8 data);
MV_STATUS mvTdmChSpiTest(MV_U32 loop);
static MV_STATUS mvTdmChSlicInit(MV_TDM_CH_INFO *chInfo);
static MV_STATUS mvTdmChSlicStart(MV_TDM_CH_INFO *chInfo);
static MV_STATUS mvTdmChSlicReset(MV_TDM_CH_INFO *chInfo);
static MV_STATUS mvTdmChTxLow(MV_TDM_CH_INFO *chInfo);
static MV_STATUS mvTdmChRxLow(MV_TDM_CH_INFO *chInfo);


MV_STATUS mvTdmInit(void)
{
	/* Trust defaults */
	return MV_OK;
}

MV_STATUS mvTdmChInit(void *osDev, MV_U8 ch, MV_U32 *tdmCh)
{
	MV_TDM_CH_INFO *chInfo;

	chInfo = (MV_TDM_CH_INFO *)mvOsMalloc(sizeof(MV_TDM_CH_INFO));
	chInfo->ch = ch;

	/* Allocate Rx buff and Tx buff */
	chInfo->rxBuff = mvOsMalloc(MV_TDM_BUFF_SIZE * 2);
	chInfo->rxBuffFull = BUFF_IS_EMPTY;

	chInfo->txBuff = mvOsMalloc(MV_TDM_BUFF_SIZE * 2);
	chInfo->txBuffFull = BUFF_IS_EMPTY;

	chInfo->active = 0;

	mvTdmChSlicInit(chInfo);

	*tdmCh = (MV_U32)chInfo;
	tdmChInfo = chInfo;

	return MV_OK;
}

MV_STATUS mvTdmChRemove(MV_U32 tdmCh)
{
	return(mvTdmChStop(tdmCh));
}


MV_STATUS mvTdmChStart(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	int i;

	mvTdmChSlicReset(chInfo);
	/*old code REG16(FPGA_CONTROL) = CONTROL_REG_MASK; */

	/* write 2 * buffer bytes size to fill the write buffer */
	for(i=0; i<MV_TDM_BUFF_SIZE; i++)
		REG16(FPGA_DRX) = 0xa5a5;

	mdelay(250);

	/* Start the SLIC */
	mvTdmChSlicStart(chInfo);

	return MV_OK;
}

MV_STATUS mvTdmChStop(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;

	/* Disable TDM interrupts */
	REG16(FPGA_CONTROL) = 0;

	/* Stop the Slic */
	mvTdmChSlicReset(chInfo);

	chInfo->active = 0;

	return MV_OK;
}

MV_STATUS mvTdmChDisable(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;

	MVDEBUG("-> %s\n",__FUNCTION__);

	if(chInfo->active)
	{
		/* enable interrupts only from SLIC */
		REG16(FPGA_CONTROL) = PROSLIC_RESET_CONTROL | SLIC_INT_MASK;

		/* wait 1/4 sec */
		mdelay(250);
	
		chInfo->active = 0;
	}

	MVDEBUG("<- %s\n",__FUNCTION__);

	return MV_OK;
}

MV_STATUS mvTdmChEnable(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;

	MVDEBUG("-> %s\n",__FUNCTION__);

	if(!chInfo->active)
	{
		/* enable interrupts */
		REG16(FPGA_CONTROL) = CONTROL_REG_MASK;

		/* wait 1/4 sec */
		mdelay(250);

		chInfo->active = 1;
	}

	MVDEBUG("<- %s\n",__FUNCTION__);

	return MV_OK;
}

/* Execute in application context (interrupt disabled) */
MV_BOOL mvTdmChTxReady(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	if(chInfo->active && (chInfo->txBuffFull == BUFF_IS_EMPTY))
		return MV_TRUE;

	return MV_FALSE;
}

/* Execute in application context (interrupt disabled) */
MV_STATUS mvTdmChTx(MV_U32 tdmCh, MV_U8 *osBuff, int count)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;

	/* Copy application data and mark buffer as full */
	if(chInfo->txBuffFull == BUFF_IS_EMPTY)
	{
		if(mvCopyFromOs(chInfo->txBuff, osBuff, count))
			return MV_FAIL;
		chInfo->txBuffFull = BUFF_IS_FULL;
		return MV_OK;
	}

	/*mvOsPrintf("%s ch%d: warning, full buff\n",__FUNCTION__,chInfo->ch);*/
	return MV_NOT_READY;
}

/* Execute in interrupt context */
static MV_STATUS mvTdmChTxLow(MV_TDM_CH_INFO *chInfo)
{
	int i;	
	unsigned short data;
	MVDEBUG("-> %s\n",__FUNCTION__);

	if(chInfo->txBuffFull == BUFF_IS_FULL)
	{
		for(i=0; i<MV_TDM_BUFF_SIZE; i+=2) 
		{
			data = ((chInfo->txBuff[i])<<8) | chInfo->txBuff[i+1];
			REG16(FPGA_DRX) = data;
		}

		chInfo->txBuffFull = BUFF_IS_EMPTY;
	}
	else
	{
		/*mvOsPrintf("%s: warning, empty buff\n",__FUNCTION__);*/
	}

	MVDEBUG("<- %s\n",__FUNCTION__);
	return MV_OK;
}

/* Execute in application context (interrupt disabled) */
MV_BOOL mvTdmChRxReady(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	if(chInfo->active && (chInfo->rxBuffFull == BUFF_IS_FULL))
		return MV_TRUE;

	return MV_FALSE;
}

/* Execute in application context (interrupt disabled) */
MV_STATUS mvTdmChRx(MV_U32 tdmCh, MV_U8 *osBuff, int count)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;

	/* Copy the data to application and mark buffer as empty */
	if(chInfo->rxBuffFull == BUFF_IS_FULL)
	{
		if(mvCopyToOs(osBuff, chInfo->rxBuff, count))
			return MV_FAIL;
		chInfo->rxBuffFull = BUFF_IS_EMPTY;
		return MV_OK;
	}
		
	/*mvOsPrintf("%s: warning, empty buff\n",__FUNCTION__);*/

	return MV_NOT_READY;
}

/* Execute in interrupt context */
static MV_STATUS mvTdmChRxLow(MV_TDM_CH_INFO *chInfo)
{
	int i;
	unsigned short data;

	MVDEBUG("-> %s\n",__FUNCTION__);

	if(chInfo->rxBuffFull == BUFF_IS_EMPTY)
	{
		for(i=0; i<MV_TDM_BUFF_SIZE; i+=2) {
			data = REG16(FPGA_DTX);
			chInfo->rxBuff[i] = (data & 0xff00) >> 8;
			chInfo->rxBuff[i+1] = data & 0xff;
		}

		chInfo->rxBuffFull = BUFF_IS_FULL;
	}
	else
	{

		/*mvOsPrintf("%s: warning, full buff\n",__FUNCTION__);*/

	}

	MVDEBUG("<- %s\n",__FUNCTION__);
	return MV_OK;
}

MV_BOOL mvTdmChExceptionReady(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	return(chInfo->exception == EXCEPTION_ON);
}

MV_STATUS mvTdmChEventGet(MV_U32 tdmCh, MV_U8 *offhook)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 tmp;

	mvTdmSpiRead(68, &tmp);

	/* hook state (0-on, 1-off) */
	*offhook = !(tmp & 4);

	chInfo->exception = EXCEPTION_OFF;

	return MV_OK;
}

MV_STATUS mvTdmIsr(void)
{
	MV_TDM_CH_INFO *chInfo = tdmChInfo;
	MV_STATUS txStatus, rxStatus;
	MV_U32 cause;

	if(tdmChInfo == NULL)
		return MV_NOT_STARTED;

	/* read cause */
	cause = REG16(FPGA_STATUS);

	/* clear read/write bits only (proslic bit is cleared by the proslic itself)           */
	/* note: we might lose an interrupt here. e.g. read bit was set between read and clear */
	REG16(FPGA_STATUS) = cause & ~(RXDBLOCK_INT_LATCH | TXDBLOCK_INT_LATCH);

	/* slic events */
	if(cause & SLIC_INT_LATCH) 
	{
		/* read and clear slic status bits */
		interruptBits();
		chInfo->exception = EXCEPTION_ON;
		return MV_OK;
	}

	MVDEBUG("cause = %x\n",cause & 0xff00);
	MVDEBUG("SYNC: write ARM=%d SLIC=%d\n", (cause & RXD_ARM_REGFLAG)?1:0, (cause & RXD_SLIC_REGFLAG)?1:0);
	MVDEBUG("SYNC: read ARM=%d SLIC=%d\n", (cause & TXD_ARM_REGFLAG)?1:0, (cause & TXD_SLIC_REGFLAG)?1:0);

#if defined(IRQ_LOOPBACK)
	doIrqLoopback(cause);
	return MV_OK;
#endif

	/* read from device - opposite bits - sync ok */
	if( (cause & TXD_SLIC_REGFLAG) ^ (cause & TXD_ARM_REGFLAG) ) 
	{
		rxStatus = mvTdmChRxLow(chInfo);
	}
	else 
	{
		rxStatus = MV_RX_ERROR;
		MVDEBUG("%s: sync rx lost\n", __FUNCTION__);
	}

	/* write to device. opposite bits - sync ok */
	if( (cause & RXD_SLIC_REGFLAG) ^ (cause & RXD_ARM_REGFLAG) ) 
	{
		txStatus = mvTdmChTxLow(chInfo);
	}
	else 
	{
		txStatus = MV_TX_ERROR;
		MVDEBUG("%s: sync tx lost\n", __FUNCTION__);
	}

	return( (rxStatus == MV_OK || txStatus == MV_OK) ? MV_OK : MV_ERROR );
}


MV_VOID mvTdmShowProperties(void)
{
	mvOsPrintf("TDM Device: on borad FPGA (single channel)\n");
}

MV_VOID mvTdmChShowProperties(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_SLIC_INFO *slic = &chInfo->slic;
	mvOsPrintf("SLIC Device: %s\n", slic->name);
}

static MV_STATUS mvTdmChSlicInit(MV_TDM_CH_INFO *chInfo)
{
	MV_U8 type;

#define MV_SI3210_FAMILY	0
#define MV_SI3210		0
#define MV_SI3211		1
#define MV_SI3210M		3
#define MV_SI3215_FAMILY	(1<<7)
#define MV_SI3215		0
#define MV_SI3215M		3

	mvTdmChSlicReset(chInfo);

	chInfo->slic.family = family();
	type = chipType();

	if(chInfo->slic.family == MV_SI3210_FAMILY)
	{
		if(type == MV_SI3210)
			strcpy(chInfo->slic.name, "SI3210");
		else if(type == MV_SI3211)
			strcpy(chInfo->slic.name, "SI3211");
		else if(type == MV_SI3210M)
			strcpy(chInfo->slic.name, "SI3210M");
		else goto _unknown;
	}
	else if(chInfo->slic.family == MV_SI3215_FAMILY)
	{
		if(type == MV_SI3215)
			strcpy(chInfo->slic.name, "SI3215");
		else if(type == MV_SI3215M)
			strcpy(chInfo->slic.name, "SI3215M");
		else goto _unknown;
	}
	else goto _unknown;

	return MV_OK;

_unknown:
	mvOsPrintf("%s: SLIC unknown",__FUNCTION__);
	strcpy(chInfo->slic.name, "Unknown");
	return MV_NOT_SUPPORTED;
}

static MV_STATUS mvTdmChSlicStart(MV_TDM_CH_INFO *chInfo)
{
	slicStart();
	enablePCMhighway ();

	/* Disable all interrupts except to on/off hook indications */
	mvTdmSpiWrite(21, 0x10);
	mvTdmSpiWrite(22, 2);
	mvTdmSpiWrite(23, 0);

	return MV_OK;
}

static MV_STATUS mvTdmChSlicReset(MV_TDM_CH_INFO *chInfo)
{
	/* reset slic and enable slic interrups */
	REG16(FPGA_CONTROL) = PROSLIC_RESET_CONTROL | SLIC_INT_MASK;
	mdelay(250);
	return MV_OK;
}

MV_STATUS mvTdmChDialTone(MV_U32 tdmCh)
{
	dialTone();
	return MV_OK;
}

MV_STATUS mvTdmChBusyTone(MV_U32 tdmCh)
{
#if 1
	busyTone();
#else
	busyJapanTone();
#endif
	return MV_OK;
}

MV_STATUS mvTdmChStopTone(MV_U32 tdmCh)
{
	stopTone();
	return MV_OK;
}

MV_STATUS mvTdmChStartRing(MV_U32 tdmCh)
{
	activateRinging();
	return MV_OK;
}

MV_STATUS mvTdmChStopRing(MV_U32 tdmCh)
{
	stopRinging();
	return MV_OK;
}

MV_STATUS mvTdmChRingBackTone(MV_U32 tdmCh)
{
#if 1
	ringBackTone();
#else
	ringBackJapanTone();
#endif
	return MV_OK;
}

static unsigned int max_count=0;

MV_STATUS mvTdmSpiWrite(MV_U8 addr, MV_U8 data)
{
	unsigned short status;
	unsigned short count = 0;

	REG16(FPGA_SPI_ACCESS) = (addr<<8) | data | SPI_WRITE_ACCESS;
	status = REG16(FPGA_SPI_READ_DATA);
	while((status & SPI_BUSY) == SPI_BUSY) {
		status = REG16(FPGA_SPI_READ_DATA);
		count++;
		if(count == 200) {
			MVDEBUG("SPI write error.\n");
			break;
		}
	}
	if(count > max_count) {
		max_count = count;
		MVDEBUG("max count = %d\n", max_count);
	}
	return MV_OK;
}

MV_STATUS mvTdmSpiRead(MV_U8 addr, MV_U8 *data)
{
	unsigned short status;
	unsigned short count = 0;

	REG16(FPGA_SPI_ACCESS) = (unsigned short)((addr<<8) | SPI_READ_ACCESS);
	status = REG16(FPGA_SPI_READ_DATA);
	while((status & SPI_BUSY) == SPI_BUSY) {
		status = REG16(FPGA_SPI_READ_DATA);
		if(++count == 200) {
			MVDEBUG("SPI read error.\n");
			break;
		}
	}
	if(count > max_count) {
		max_count = count;
		MVDEBUG("max count = %d\n", max_count);
	}

	*data = (status & 0xff);
	return MV_OK;
}

MV_STATUS mvTdmChSpiTest(MV_U32 loop)
{
	volatile unsigned short w,r;
	int i;

	REG16(FPGA_STATUS) = 0;

	MVDEBUG("SPI Write/Read test %d loops... ",loop);
	for(i=0;i<loop;i++) 
	{
		w = (i & 0x7f);
		r = 0;
		writeDirectReg(2, w);
		r = readDirectReg(2);
		if(r!=w) 
		{
			MVDEBUG("SPI: Wrote %x, Read = %x ",w,r);
			break;
		}
	}

	REG16(FPGA_STATUS) = CONTROL_REG_MASK;

	if(i != loop)
	{
		MVDEBUG("failed\n");
		return MV_FAIL;
	}

	MVDEBUG("ok\n");
	return MV_OK;
}

