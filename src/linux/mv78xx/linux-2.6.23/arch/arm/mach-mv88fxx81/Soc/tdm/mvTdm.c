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
#include "mvTdm.h"
#include "mvTdmRegs.h"
#include "mvDramIf.h"
#include "mvCtrlEnvAddrDec.h"
#include "proslic.h"
#include "mvOs.h"

#include "dbg-trace.h"

/* TDM configuration */
#define CH0_RX_SLOT	0
#define CH0_TX_SLOT	0
#define CH1_RX_SLOT	10
#define CH1_TX_SLOT	10

#ifdef MV_TDM_LINEAR_MODE
#define PCM_SAMPLE_SIZE PCM_SAMPLE_SIZE_2
#else
#define PCM_SAMPLE_SIZE PCM_SAMPLE_SIZE_1
#endif

#define TOTAL_SAMPLE	80
#define INT_SAMPLE	2

#define TDM_INT_SLIC	(DMA_ABORT_BIT|SLIC_INT_BIT)
#define TDM_INT_TX(ch)	(TX_UNDERFLOW_BIT(ch)|TX_BIT(ch)|TX_IDLE_BIT(ch))
#define TDM_INT_RX(ch)	(RX_OVERFLOW_BIT(ch)|RX_BIT(ch)|RX_IDLE_BIT(ch))

#define CONFIG_PCM_CRTL (MASTER_PCLK_TDM | MASTER_FS_TDM |DATA_POLAR_NEG | \
			FS_POLAR_NEG | INVERT_FS_HI | FS_TYPE_SHORT	 | \
			PCM_SAMPLE_SIZE | CH_DELAY_DISABLE 		 | \
			CH_QUALITY_DISABLE | QUALITY_POLARITY_NEG	 | \
			QUALITY_TYPE_TIME_SLOT | CS_CTRL_DONT_CARE 	 | \
			WIDEBAND_OFF | PERF_GBUS_TWO_ACCESS)

#define CONFIG_TIMESLOT_CTRL ((CH0_RX_SLOT<<CH0_RX_SLOT_OFFS) | \
			      (CH0_TX_SLOT<<CH0_TX_SLOT_OFFS) | \
			      (CH1_RX_SLOT<<CH1_RX_SLOT_OFFS) | \
			      (CH1_TX_SLOT<<CH1_TX_SLOT_OFFS))

#define CONFIG_CH_SAMPLE ((TOTAL_SAMPLE<<TOTAL_CNT_OFFS) | (INT_SAMPLE<<INT_CNT_OFFS))

/* Helpers */
#define BUFF_IS_FULL	1
#define BUFF_IS_EMPTY	0
#define EXCEPTION_ON	1
#define EXCEPTION_OFF	0

/* SLIC and TDM structures */
typedef struct _mv_slic_info
{
	MV_U8 family;
	MV_U8 name[32];
	MV_U8 cs;
} MV_SLIC_INFO;

typedef struct _mv_tdm_ch_info
{
	MV_U8 ch;
	MV_U8 exception;
	MV_U8 rxActive, txActive;
	MV_U8 *rxBuffVirt[2], *txBuffVirt[2];
	MV_ULONG rxBuffPhys[2], txBuffPhys[2];
	MV_U8 rxBuffFull[2], txBuffFull[2];
	MV_U8 rxCurrBuff, txCurrBuff;
	MV_SLIC_INFO slic;
} MV_TDM_CH_INFO;

/* Forwards */
static MV_TDM_CH_INFO *tdmChInfo[MV_TDM_MAX_CHANNELS];
static MV_STATUS mvTdmChSlicInit(MV_TDM_CH_INFO *chInfo);
static MV_STATUS mvTdmChTxLow(MV_TDM_CH_INFO *chInfo);
static MV_STATUS mvTdmChRxLow(MV_TDM_CH_INFO *chInfo);
static void setDaisyChainMode(void);
static void setCurrentSlic(MV_SLIC_INFO *slic);
static MV_STATUS mvTdmSlicReset(void);
MV_STATUS mvTdmSpiRead(MV_U8 addr, MV_U8 *data);
MV_STATUS mvTdmSpiWrite(MV_U8 addr, MV_U8 data);
MV_STATUS mvTdmChSpiTest(MV_U8 loop);


MV_STATUS mvTdmInit(void)
{
	TRC_REC("->%s\n",__FUNCTION__);

	/* Config TDM */
	MV_REG_BIT_RESET(TDM_SPI_MUX_REG, 1);                       /* enable TDM/SPI interface */
	if(mvCtrlModelRevGet() == MV_5181L_A1_ID)
		MV_REG_BIT_SET(TDM_MISC_REG, BIT0);             /* sw reset to TDM for 5181L-A1 */
	MV_REG_WRITE(INT_RESET_SELECT_REG,CLEAR_ON_ZERO);     /* int cause is not clear on read */
	MV_REG_WRITE(INT_EVENT_MASK_REG,0x3ffff);       /* all interrupt bits latched in status */
	MV_REG_WRITE(INT_STATUS_MASK_REG,0);                              /* disable interrupts */
	MV_REG_WRITE(INT_STATUS_REG,0);                            /* clear int status register */
	MV_REG_WRITE(PCM_CTRL_REG, CONFIG_PCM_CRTL);                       /* PCM configuration */
	MV_REG_WRITE(TIMESLOT_CTRL_REG, CONFIG_TIMESLOT_CTRL);      /* channels rx/tx timeslots */
	MV_REG_WRITE(PCM_CLK_RATE_DIV_REG, PCM_8192KHZ);                       /* PCM PCLK freq */
	MV_REG_WRITE(FRAME_TIMESLOT_REG, TIMESLOTS128_8192KHZ);   /* Number of timeslots (PCLK) */
	MV_REG_WRITE(DUMMY_RX_WRITE_DATA_REG,0);                    /* Padding on Rx completion */
	MV_REG_BYTE_WRITE(SPI_GLOBAL_CTRL_REG, MV_REG_READ(SPI_GLOBAL_CTRL_REG) | SPI_GLOBAL_ENABLE);

	/* Reset SLIC(s) */
	mvTdmSlicReset();

#ifdef MV_TDM_DAISY_CHAIN
	/* Configure SLIC(s) to work in daisy chain mode */
	TRC_REC("configure daisy chain mode\n");
	setDaisyChainMode();
#endif

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChInit(void *osDev, MV_U8 ch, MV_U32 *tdmCh)
{
	MV_TDM_CH_INFO *chInfo;
	MV_U8 buff;

	TRC_REC("->%s ch%d\n",__FUNCTION__,ch);

	if(ch+1 > MV_TDM_MAX_CHANNELS)
	{
		mvOsPrintf("%s: error, support only two SLIC devices\n",__FUNCTION__);
		return MV_BAD_PARAM;
	}

	tdmChInfo[ch] = chInfo = (MV_TDM_CH_INFO *)mvOsMalloc(sizeof(MV_TDM_CH_INFO));
	if(!chInfo)
	{
		mvOsPrintf("%s: error malloc failed\n",__FUNCTION__);
		return MV_NO_RESOURCE;
	}

	chInfo->ch = ch;

	/* Per channel TDM init */
	MV_REG_WRITE(CH_ENABLE_REG(ch),CH_DISABLE);  /* disbale channel (enable in channel start) */
	MV_REG_WRITE(CH_SAMPLE_REG(ch),CONFIG_CH_SAMPLE); /* set total samples and the int sample */

	/* Per SLIC channel init */
	if(mvTdmChSlicInit(chInfo) != MV_OK)
	{
		mvOsPrintf("%s: error, SLIC device not found\n",__FUNCTION__);
		return MV_NOT_FOUND;
	}

	for(buff=0; buff<2; buff++)
	{
		/* Buffers must be 32B aligned */
		chInfo->rxBuffVirt[buff] = (MV_U8*)mvOsIoUncachedMalloc(NULL, MV_TDM_BUFF_SIZE, &(chInfo->rxBuffPhys[buff]));
		chInfo->rxBuffFull[buff] = BUFF_IS_EMPTY;

		chInfo->txBuffVirt[buff] = (MV_U8*)mvOsIoUncachedMalloc(NULL, MV_TDM_BUFF_SIZE, &(chInfo->txBuffPhys[buff]));
		chInfo->txBuffFull[buff] = BUFF_IS_EMPTY;

		if(((MV_ULONG)chInfo->rxBuffVirt[buff] | chInfo->rxBuffPhys[buff] | 
			(MV_ULONG)chInfo->txBuffVirt[buff] | chInfo->txBuffPhys[buff]) & 0x1f) {
			mvOsPrintf("%s: error, unaligned buffer allocation\n", __FUNCTION__);
		}
	}

	chInfo->rxActive = 0;
	chInfo->txActive = 0;

	/* Pass chInfo cookie for upper level */
	*tdmCh = (MV_U32)chInfo;

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChRemove(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 buff;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	mvTdmChStop(tdmCh);

	for(buff=0; buff<2; buff++)
	{
		mvOsIoUncachedFree(NULL, MV_TDM_BUFF_SIZE, chInfo->rxBuffPhys[buff], chInfo->rxBuffVirt[buff]);
		mvOsIoUncachedFree(NULL, MV_TDM_BUFF_SIZE, chInfo->txBuffPhys[buff], chInfo->txBuffVirt[buff]);
	}

	mvOsFree(chInfo);

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

/* Start: only SLIC interrupts are enabled */
MV_STATUS mvTdmChStart(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	/* Enable SLIC on/off hook interrupts */
	TRC_REC("enable slic%d interrupts\n",chInfo->ch);
	setCurrentSlic(&chInfo->slic);
	mvTdmSpiWrite(21, 0x10);
	mvTdmSpiWrite(22, 2);
	mvTdmSpiWrite(23, 0);

	MV_REG_WRITE(INT_STATUS_REG,0); 
	MV_REG_WRITE(INT_STATUS_MASK_REG,TDM_INT_SLIC); 

	chInfo->rxActive = chInfo->txActive = 0;
	chInfo->rxBuffFull[0] = chInfo->txBuffFull[0] = BUFF_IS_EMPTY;
	chInfo->rxBuffFull[1] = chInfo->txBuffFull[1] = BUFF_IS_EMPTY;

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

/* Stop: everything is disabled */
MV_STATUS mvTdmChStop(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	/* Disable Rx/Tx */
	mvTdmChRxDisable(tdmCh);
	mvTdmChTxDisable(tdmCh);

	/* Clear cause and disable all interrupts */
	TRC_REC("disable all ch%d interrupts\n",chInfo->ch);
	MV_REG_WRITE(INT_STATUS_MASK_REG,0);
	MV_REG_WRITE(INT_STATUS_REG,0);

	chInfo->rxActive = chInfo->txActive = 0;
	chInfo->rxBuffFull[0] = chInfo->txBuffFull[0] = BUFF_IS_EMPTY;
	chInfo->rxBuffFull[1] = chInfo->txBuffFull[1] = BUFF_IS_EMPTY;

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChRxEnable(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	if(!chInfo->rxActive)
	{
		chInfo->rxActive = 1;
		chInfo->rxCurrBuff = 0;
		chInfo->rxBuffFull[0] = BUFF_IS_EMPTY;
		chInfo->rxBuffFull[1] = BUFF_IS_EMPTY;

		/* Enable also RX interrupts */
		MV_REG_WRITE( INT_STATUS_REG, MV_REG_READ(INT_STATUS_REG) & (~(TDM_INT_RX(chInfo->ch))) );
		MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) | TDM_INT_RX(chInfo->ch));

		/* Set RX buff */
		MV_REG_WRITE(CH_RX_ADDR_REG(chInfo->ch),chInfo->rxBuffPhys[chInfo->rxCurrBuff]);
		MV_REG_BYTE_WRITE(CH_BUFF_OWN_REG(chInfo->ch)+RX_OWN_BYTE_OFFS, OWN_BY_HW);

		/* Enable RX */
		MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+RX_ENABLE_BYTE_OFFS, CH_ENABLE);
	}
	else
	{
		TRC_REC("rx already active\n");
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChRxDisable(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U32 max_poll = 0;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	if(chInfo->rxActive)
	{
		chInfo->rxActive = 0;
		chInfo->rxCurrBuff = 0;
		chInfo->rxBuffFull[0] = BUFF_IS_EMPTY;
		chInfo->rxBuffFull[1] = BUFF_IS_EMPTY;

		/* Disable RX */
		MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) & (~(TDM_INT_RX(chInfo->ch)))); 
		MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+RX_ENABLE_BYTE_OFFS, CH_DISABLE);

		/* Poll on idle indication (timeout 15ms) */
		/* Tzachi - we always exit on timeout. why idle bit is'nt asserted? */
		TRC_REC("%s ch%d start poll for rx idle\n",__FUNCTION__,chInfo->ch);
		while( (!(MV_REG_READ(INT_STATUS_REG) & RX_IDLE_BIT(chInfo->ch))) && (max_poll < 15000) )
		{
			udelay(1);
			max_poll++;
		}
		if(max_poll == 15000)
		{
			TRC_REC("%s ch%d poll timeout (~15ms)\n",__FUNCTION__,chInfo->ch);
		}
		else
		{
			TRC_REC("%s ch%d poll stop ok\n",__FUNCTION__,chInfo->ch);
		}

		/* Clear rx bits in cause register */
		MV_REG_WRITE(INT_STATUS_REG, MV_REG_READ(INT_STATUS_REG) & (~(TDM_INT_RX(chInfo->ch)))); 
	}
	else
	{
		TRC_REC("rx already not active\n");
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChTxEnable(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	if(!chInfo->txActive)
	{
		chInfo->txActive = 1;
		chInfo->txCurrBuff = 0;
		chInfo->txBuffFull[0] = BUFF_IS_EMPTY;
		chInfo->txBuffFull[1] = BUFF_IS_EMPTY;

		/* Enable also TX interrupts */
		MV_REG_WRITE( INT_STATUS_REG, MV_REG_READ(INT_STATUS_REG) & (~(TDM_INT_TX(chInfo->ch))) );
		MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) | TDM_INT_TX(chInfo->ch)); 

		/* Set TX buff */
		MV_REG_WRITE(CH_TX_ADDR_REG(chInfo->ch),chInfo->txBuffPhys[chInfo->txCurrBuff]);
		MV_REG_BYTE_WRITE(CH_BUFF_OWN_REG(chInfo->ch)+TX_OWN_BYTE_OFFS, OWN_BY_HW);

		/* Enable TX */
		MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+TX_ENABLE_BYTE_OFFS, CH_ENABLE);
	}
	else
	{
		TRC_REC("tx already active\n");
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChTxDisable(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U32 max_poll = 0;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	if(chInfo->txActive)
	{
		chInfo->txActive = 0;
		chInfo->txCurrBuff = 0;
		chInfo->txBuffFull[0] = BUFF_IS_EMPTY;
		chInfo->txBuffFull[1] = BUFF_IS_EMPTY;

		/* Disable TX */
		MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) & (~(TDM_INT_TX(chInfo->ch)))); 
		MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+TX_ENABLE_BYTE_OFFS, CH_DISABLE);

		/* Poll on idle indication (timeout 15ms) */
		/* Tzachi - we always exit on timeout. why idle bit is'nt asserted? */
		TRC_REC("%s ch%d start poll for tx idle\n",__FUNCTION__,chInfo->ch);
		while( (!(MV_REG_READ(INT_STATUS_REG) & RX_IDLE_BIT(chInfo->ch))) && (max_poll < 15000) )
		{
			udelay(1);
			max_poll++;
		}
		if(max_poll == 15000)
		{
			TRC_REC("%s ch%d poll timeout (~15ms)\n",__FUNCTION__,chInfo->ch);
		}
		else
		{
			TRC_REC("%s ch%d poll stop ok\n",__FUNCTION__,chInfo->ch);
		}

		/* Clear tx bits in cause register */
		MV_REG_WRITE(INT_STATUS_REG, MV_REG_READ(INT_STATUS_REG) & (~(TDM_INT_TX(chInfo->ch)))); 
	}
	else
	{
		TRC_REC("tx already not active\n");
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_BOOL mvTdmChTxReady(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 nextBuff = !chInfo->txCurrBuff;
	if(chInfo->txActive && (chInfo->txBuffFull[nextBuff] == BUFF_IS_EMPTY))
		return MV_TRUE;
	return MV_FALSE;
}

MV_BOOL mvTdmChRxReady(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 lastBuff = !chInfo->rxCurrBuff;
	if(chInfo->rxActive && (chInfo->rxBuffFull[lastBuff] == BUFF_IS_FULL))
		return MV_TRUE;
	return MV_FALSE;
}

MV_BOOL mvTdmChExceptionReady(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	return(chInfo->exception == EXCEPTION_ON);
}

/* TX: Copy the data into the next frame buff to be transmitted */
MV_STATUS mvTdmChTx(MV_U32 tdmCh, MV_U8 *osBuff, int count)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 currBuff, nextBuff;
	MV_STATUS ret;

	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	if(chInfo->txActive)
	{
		currBuff = chInfo->txCurrBuff;
		nextBuff = !currBuff;

		count = (count <= MV_TDM_BUFF_SIZE) ? count : MV_TDM_BUFF_SIZE;

		/* Copy application data and mark buffer as full */
		if(chInfo->txBuffFull[nextBuff] == BUFF_IS_EMPTY)
		{
			if(mvCopyFromOs(chInfo->txBuffVirt[nextBuff], osBuff, count))
				return MV_FAIL;

			chInfo->txBuffFull[nextBuff] = BUFF_IS_FULL;
			ret = MV_OK;
		}
		else
		{
			TRC_REC("app find next tx buff full [irq not ok]\n");
			ret = MV_NOT_READY;
		}
	}
	else
	{
		TRC_REC("tx is not active!\n");
		ret = MV_NOT_READY;
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return ret; 
}

/* RX: Copy the data from the last recieved buff to application and mark it as empty */
MV_STATUS mvTdmChRx(MV_U32 tdmCh, MV_U8 *osBuff, int count)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 currBuff, lastBuff;
	MV_STATUS ret;

	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	if(chInfo->rxActive)
	{
		currBuff = chInfo->rxCurrBuff;
		lastBuff = !currBuff;

		count = ((count <= MV_TDM_BUFF_SIZE) ? count : MV_TDM_BUFF_SIZE);

		/* Copy the data to application and mark buffer as empty */
		if(chInfo->rxBuffFull[lastBuff] == BUFF_IS_FULL)
		{
			if(mvCopyToOs(osBuff, chInfo->rxBuffVirt[lastBuff], count))
				return MV_FAIL;

			chInfo->rxBuffFull[lastBuff] = BUFF_IS_EMPTY;
			ret = MV_OK;
		}
		else
		{
			TRC_REC("app find last rx buff empty [irq not ok]\n");
			ret = MV_NOT_READY;
		}
	}
	else
	{
		TRC_REC("rx is not active!\n");
		ret = MV_NOT_READY;
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return ret;
}

MV_STATUS mvTdmChEventGet(MV_U32 tdmCh, MV_U8 *offhook)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 tmp;

	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	setCurrentSlic(&chInfo->slic);
	mvTdmSpiRead(68, &tmp);

	/* hook state (0-on, 1-off) */
	*offhook = !(tmp & 4);

	if(*offhook)
		TRC_REC("off-hook\n");
	else
		TRC_REC("on-hook\n");

	chInfo->exception = EXCEPTION_OFF;

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmIsr(void)
{
	MV_TDM_CH_INFO *chInfo;
	MV_U8 ch = 0;
	MV_U32 cause;
	MV_U32 work_done = 0;

	/* Read and clear cause */
	cause = MV_REG_READ(INT_STATUS_REG);
	MV_REG_WRITE(INT_STATUS_REG, ~cause);

	TRC_REC("->%s cause=%x mask=%x\n",__FUNCTION__,cause,MV_REG_READ(INT_STATUS_MASK_REG));

	/* Refer only to unmasked bits */
	cause &= (MV_REG_READ(INT_STATUS_MASK_REG));

	if(cause & SLIC_INT_BIT)
	{
		for(ch=0; ch<MV_TDM_MAX_CHANNELS; ch++)
		{
			chInfo = tdmChInfo[ch];
			setCurrentSlic(&chInfo->slic);
			if(interruptBits()) {
				chInfo->exception = EXCEPTION_ON;
				TRC_REC("ch%d slic interrupt\n",ch);
			}
		}
		work_done |= 1;
	}

	if(cause & DMA_ABORT_BIT)
	{
		mvOsPrintf("%s: DMA data abort. Address: 0x%08x, Info: 0x%08x\n",
		__FUNCTION__, MV_REG_READ(DMA_ABORT_ADDR_REG), MV_REG_READ(DMA_ABORT_INFO_REG));
	}

	for(ch=0; ch<MV_TDM_MAX_CHANNELS; ch++)
	{
		chInfo = tdmChInfo[ch];

		if(cause & TDM_INT_RX(ch))
		{
/*			if(cause & RX_BIT(ch)) 
				{TRC_REC("ch%d rx interrupt\n",ch);}
			if(cause & RX_OVERFLOW_BIT(ch))
				{TRC_REC("ch%d rx overflow interrupt\n",ch);}
			if(cause & RX_IDLE_BIT(ch))
				{TRC_REC("ch%d rx idle interrupt\n",ch);}
*/
			if(chInfo->rxActive)
			{
				/* Give next buff to TDM and set curr buff as full */
				mvTdmChRxLow(chInfo);
				work_done |= 1;
			}
		}

		if(cause & TDM_INT_TX(ch))
		{
/*			if(cause & TX_BIT(ch))
				{TRC_REC("ch%d tx interrupt\n",ch);}
			if(cause & TX_UNDERFLOW_BIT(ch))
				{TRC_REC("ch%d tx underflow interrupt\n",ch);}
			if(cause & TX_IDLE_BIT(ch))
				{TRC_REC("ch%d tx idle interrupt\n",ch);}
*/
			if(chInfo->txActive)
			{
				/* Give next buff to TDM and set curr buff as empty */
				mvTdmChTxLow(chInfo);
				work_done |= 1;
			}
		}
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return( (work_done) ? MV_OK : MV_NOT_READY );
}

/*
** Called from ISR 
** Mark last frame transmitted buff as empty and give current full buff to HW
** Write procedure mark last frame buff as full after filling it with data
*/
static MV_STATUS mvTdmChTxLow(MV_TDM_CH_INFO *chInfo)
{
	MV_U8 currBuff, lastBuff;
	MV_U32 max_poll = 0;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	/* Change buffers */
	chInfo->txCurrBuff = !chInfo->txCurrBuff;

	currBuff = chInfo->txCurrBuff;
	lastBuff = !currBuff;

	if(chInfo->txBuffFull[currBuff] == BUFF_IS_FULL)
	{
		TRC_REC("curr buff full for hw [app ok]\n");
	}
	else
	{
		TRC_REC("curr buf is empty [app miss write]\n");
	}

	/* Mark last buff that was transmitted by HW as empty. Give it back to HW */
	/* for next frame. The app need to write the data before HW takes it.     */
	memset(chInfo->txBuffVirt[lastBuff], 0, MV_TDM_BUFF_SIZE);
	chInfo->txBuffFull[lastBuff] = BUFF_IS_EMPTY;

	/* Poll on SW ownership (single check) */
	TRC_REC("start poll for SW ownership\n");
	while( ((MV_REG_BYTE_READ(CH_BUFF_OWN_REG(chInfo->ch)+TX_OWN_BYTE_OFFS) & OWNER_MASK) == OWN_BY_HW) 
		&& (max_poll < 2000) )
	{
		udelay(1);
		max_poll++;
	}
	if(max_poll == 2000) 
	{
		TRC_REC("poll timeout (~2ms)\n");
		TRC_REC("<-giving up this buff\n");
		return MV_TIMEOUT;
	}
	else
	{
		TRC_REC("tx-low poll stop ok\n");
	}

	/* Set TX buff address (must be 32 byte aligned) */
	MV_REG_WRITE(CH_TX_ADDR_REG(chInfo->ch),chInfo->txBuffPhys[lastBuff]);

	/* Set HW ownership */
	MV_REG_BYTE_WRITE(CH_BUFF_OWN_REG(chInfo->ch)+TX_OWN_BYTE_OFFS, OWN_BY_HW);

	/* Enable Tx (do I need to do it every time?) */
	MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+TX_ENABLE_BYTE_OFFS, CH_ENABLE);

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

/* 
** Called from Isr 
** Mark last frame recieved buff as full and give current (empty) buff to HW
** The read procedure will mark the full (last frame) buff as empty after copying the data
*/
static MV_STATUS mvTdmChRxLow(MV_TDM_CH_INFO *chInfo)
{
	MV_U8 currBuff, lastBuff;
	MV_U32 max_poll = 0;
	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	/* Change buffers */
	chInfo->rxCurrBuff = !chInfo->rxCurrBuff;

	currBuff = chInfo->rxCurrBuff;
	lastBuff = !currBuff;

	if(chInfo->rxBuffFull[lastBuff] == BUFF_IS_EMPTY)
	{
		TRC_REC("curr buff empty for hw [app ok]\n");
	}
	else
	{
		TRC_REC("curr buf is full [app miss read]\n");
	}
	
	/* Mark last buff that was received by HW as full. Give it back to HW for */
	/* next frame. The app need to read the data before HW put new data on it.*/
	chInfo->rxBuffFull[lastBuff] = BUFF_IS_FULL;

	/* Poll on SW ownership (single check) */
	TRC_REC("start poll for ownership\n");
	while( ((MV_REG_BYTE_READ(CH_BUFF_OWN_REG(chInfo->ch)+RX_OWN_BYTE_OFFS) & OWNER_MASK) == OWN_BY_HW) 
		&& (max_poll < 2000) )
	{
		udelay(1);
		max_poll++;
	}
	if(max_poll == 2000)
	{
		TRC_REC("poll timeout (~2ms)\n");
		TRC_REC("<-giving up this buff\n");
		return MV_TIMEOUT;
	}
	else
	{
		TRC_REC("poll stop ok\n");
	}

	/* Set RX buff address (must be 32 byte aligned) */
	MV_REG_WRITE(CH_RX_ADDR_REG(chInfo->ch),chInfo->rxBuffPhys[lastBuff]);

	/* Set HW ownership */
	MV_REG_BYTE_WRITE(CH_BUFF_OWN_REG(chInfo->ch)+RX_OWN_BYTE_OFFS, OWN_BY_HW);

	/* Enable Rx (do I need to do it every time?) */
	MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+RX_ENABLE_BYTE_OFFS, CH_ENABLE);
	
	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

/*******************************
**        SLIC Helpers        **
*******************************/
static MV_STATUS mvTdmChSlicInit(MV_TDM_CH_INFO *chInfo)
{
	MV_U8 type;
	MV_U16 txSample, rxSample;

	TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

#define MV_SI3210_FAMILY	0
#define MV_SI3210		0
#define MV_SI3211		1
#define MV_SI3210M		3
#define MV_SI3215_FAMILY	(1<<7)
#define MV_SI3215		0
#define MV_SI3215M		3

#ifdef MV_TDM_DAISY_CHAIN
{
	MV_U8 val;

	/* First set the Chip Select byte for daisy chain mode */
	chInfo->slic.cs = chInfo->ch + 1;

	TRC_REC("ch%d chip select byte = %d\n",chInfo->slic.cs);

	setCurrentSlic(&chInfo->slic);
	mvTdmSpiRead(0,&val);
	if(!(val & 0x80))
	{
		mvOsPrintf("error, ch %d is not in daisy chain\n",chInfo->ch);
		return MV_ERROR;
	}
}
#endif

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

	/* Reset init */
	TRC_REC("start slic%d\n",chInfo->ch);
	slicStart();
	enablePCMhighway();

	/* Disable all interrupts from SLIC */
	mvTdmSpiWrite(21, 0);
	mvTdmSpiWrite(22, 0);
	mvTdmSpiWrite(23, 0);

#ifdef MV_TDM_LINEAR_MODE
{
	/* Configure linear mode */
	MV_U8 val;
	mvOsPrintf("SLIC%d: using linear mode\n",chInfo->ch);
	mvTdmSpiRead(1, &val);
	mvTdmSpiWrite(1,val|(0x3<<3)|(1<<2)); /* linear mode + 16 bit transfer */
}
#endif
	/* Configure tx/rx sample in SLIC */
	txSample = ((chInfo->ch==0) ? CH0_TX_SLOT : CH1_TX_SLOT);
	rxSample = ((chInfo->ch==0) ? CH0_RX_SLOT : CH1_RX_SLOT);
	mvOsPrintf("SLIC%d: RX sample %d, TX sample %d\n",chInfo->ch,rxSample,txSample);
	txSample *= 8;
	rxSample *= 8;
	mvTdmSpiWrite(2,txSample&0xff);
	mvTdmSpiWrite(3,(txSample>>8)&0x3);
	mvTdmSpiWrite(4,rxSample&0xff);
	mvTdmSpiWrite(5,(rxSample>>8)&0x3);

	TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;

_unknown:
	strcpy(chInfo->slic.name, "Unknown");
	return MV_NOT_SUPPORTED;
}

static MV_STATUS mvTdmSlicReset(void)
{
	TRC_REC("reseting slic(s)\n");
	MV_REG_WRITE(MISC_CTRL_REG,0);
	mvOsDelay(250);
	MV_REG_WRITE(MISC_CTRL_REG,1);
	mvOsDelay(250);
	return MV_OK;
}

MV_STATUS mvTdmChDialTone(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	setCurrentSlic(&chInfo->slic);
	dialTone();
	return MV_OK;
}

MV_STATUS mvTdmChBusyTone(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	setCurrentSlic(&chInfo->slic);
#if 1
	busyTone();
#else
	busyJapanTone();
#endif
	return MV_OK;
}

MV_STATUS mvTdmChStopTone(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	setCurrentSlic(&chInfo->slic);
	stopTone();
	return MV_OK;
}

MV_STATUS mvTdmChStartRing(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	setCurrentSlic(&chInfo->slic);
	activateRinging();
	return MV_OK;
}

MV_STATUS mvTdmChStopRing(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	setCurrentSlic(&chInfo->slic);
	stopRinging();
	return MV_OK;
}

MV_STATUS mvTdmChRingBackTone(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	setCurrentSlic(&chInfo->slic);
#if 1
	ringBackTone();
#else
	ringBackJapanTone();
#endif
	return MV_OK;
}

/****************************
**        SPI Stuff        **
****************************/
static MV_SLIC_INFO *currentSlic = NULL;

static void setCurrentSlic(MV_SLIC_INFO *slic)
{
//	TRC_REC("%s: slic cs %d\n",__FUNCTION__,slic->cs);
	currentSlic = slic;
}

#ifdef MV_TDM_DAISY_CHAIN
static void setDaisyChainMode(void)
{
	mvOsPrintf("Setting Daisy Chain Mode\n");
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);
	MV_REG_WRITE(SPI_CODEC_CMD_LO_REG, (0x80<<8) | 0);
	MV_REG_WRITE(SPI_CODEC_CTRL_REG, TRANSFER_BYTES(2) | ENDIANESS_MSB_MODE | WR_MODE | CLK_SPEED_LO_DIV);
	MV_REG_WRITE(SPI_CTRL_REG, MV_REG_READ(SPI_CTRL_REG) | SPI_ACTIVE);
	mvOsDelay(50);
}
#endif

MV_STATUS mvTdmSpiWrite(MV_U8 addr, MV_U8 data)
{
	MV_U32 val1, val2;

#ifdef MV_TDM_DAISY_CHAIN
	if(currentSlic == NULL) {
		mvOsPrintf("%s: slic is not initialized\n",__FUNCTION__);
		return MV_NOT_INITIALIZED;
	}
#endif

//	TRC_REC("spi write: cs=%x reg=%d data=%x\n",currentSlic->cs,addr,data);

	if(addr & 0x80)
		mvOsPrintf("%s: error, writing with read bit on (addr=0x%x)\n",__FUNCTION__,addr);

	/* Poll for ready indication */
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);

	/* Configure transaction */
#ifdef MV_TDM_DAISY_CHAIN
	val1 = (addr<<8) | currentSlic->cs;
	MV_REG_WRITE(SPI_CODEC_CMD_LO_REG, val1);
	MV_REG_WRITE(SPI_CODEC_CMD_HI_REG, data);
	val2 = TRANSFER_BYTES(3) | ENDIANESS_MSB_MODE | WR_MODE | CLK_SPEED_LO_DIV;
#else
	val1 = (data<<8) | addr;
	MV_REG_WRITE(SPI_CODEC_CMD_LO_REG, val1);
	val2 = TRANSFER_BYTES(2) | ENDIANESS_MSB_MODE | WR_MODE | CLK_SPEED_LO_DIV;
#endif

	MV_REG_WRITE(SPI_CODEC_CTRL_REG, val2);

	/* Activate */
	MV_REG_WRITE(SPI_CTRL_REG, MV_REG_READ(SPI_CTRL_REG) | SPI_ACTIVE);

	/* Poll for ready indication */
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);

	return MV_OK;
}

MV_STATUS mvTdmSpiRead(MV_U8 addr, MV_U8 *data)
{
	MV_U32 val1, val2;

#ifdef MV_TDM_DAISY_CHAIN
	if(currentSlic == NULL) {
		mvOsPrintf("%s: slic is not initialized\n",__FUNCTION__);
		return MV_NOT_INITIALIZED;
	}
#endif

//	TRC_REC("spi read: cs=%x reg=%d... ",currentSlic->cs,addr);

	addr |= 0x80;

	/* Poll for ready indication */
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);

	/* Configure transaction */
#ifdef MV_TDM_DAISY_CHAIN
	val1 = (addr<<8) | currentSlic->cs;
	val2 = TRANSFER_BYTES(2) | ENDIANESS_MSB_MODE | RD_MODE | READ_1_BYTE | CLK_SPEED_LO_DIV;
#else
	val1 = addr;
	val2 = TRANSFER_BYTES(1) | ENDIANESS_MSB_MODE | RD_MODE | READ_1_BYTE | CLK_SPEED_LO_DIV;
#endif
	MV_REG_WRITE(SPI_CODEC_CMD_LO_REG, val1);
	MV_REG_WRITE(SPI_CODEC_CTRL_REG, val2);

	/* Activate */
	MV_REG_WRITE(SPI_CTRL_REG, MV_REG_READ(SPI_CTRL_REG) | SPI_ACTIVE);

	/* Poll for ready indication */
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);

	*data = MV_REG_BYTE_READ(SPI_CODEC_READ_DATA_REG);

//	TRC_REC(" data=0x%x\n",*data);
	return MV_OK;
}

MV_STATUS mvTdmChSpiTest(MV_U8 loop)
{
	MV_TDM_CH_INFO *chInfo;
	volatile unsigned short w,r;
	int i,ch;

	for(ch=0; ch<MV_TDM_MAX_CHANNELS; ch++)
	{
		chInfo = tdmChInfo[ch];

		/* Set current SLIC device */
		setCurrentSlic(&chInfo->slic);

		mvOsPrintf("SPI channel %d Write/Read test %d loops... ",chInfo->ch,loop);

		for(i=0;i<loop;i++) 
		{
			w = (i & 0x7f);
			r = 0;
			mvTdmSpiWrite(2, w);
			r = readDirectReg(2);
			if(r!=w) 
			{
				mvOsPrintf("SPI: Wrote %x, Read = %x ",w,r);
				break;
			}
		}

		if(i != loop)
		{
			mvOsPrintf("failed\n");
			return MV_FAIL;
		}

		mvOsPrintf("ok\n");
	}
	return MV_OK;
}

/******************
** Debug Display **
******************/
MV_VOID mvOsRegDump(MV_U32 reg)
{
	mvOsPrintf("0x%05x: %08x\n",reg,MV_REG_READ(reg));
}

MV_VOID mvTdmRegsDump(void)
{
	MV_U8 i;
	MV_TDM_CH_INFO *chInfo;

	mvOsPrintf("TDM Address Decode Windows:\n");
	for(i=0;i<TDM_MBUS_MAX_WIN;i++)
	{
		mvOsRegDump(TDM_WIN_CTRL_REG(i));
		mvOsRegDump(TDM_WIN_BASE_REG(i));
	}
	mvOsPrintf("TDM Control:\n");
	mvOsRegDump(TDM_SPI_MUX_REG);
	mvOsRegDump(INT_RESET_SELECT_REG);
	mvOsRegDump(INT_STATUS_MASK_REG);
	mvOsRegDump(INT_STATUS_REG);
	mvOsRegDump(INT_EVENT_MASK_REG);
	mvOsRegDump(PCM_CTRL_REG);
	mvOsRegDump(TIMESLOT_CTRL_REG);
	mvOsRegDump(PCM_CLK_RATE_DIV_REG);
	mvOsRegDump(FRAME_TIMESLOT_REG);
	mvOsRegDump(DUMMY_RX_WRITE_DATA_REG);
	mvOsRegDump(MISC_CTRL_REG);
	mvOsPrintf("TDM Channel Control:\n");
	for(i=0;i<MV_TDM_MAX_CHANNELS;i++)
	{
		mvOsRegDump(CH_DELAY_CTRL_REG(i));
		mvOsRegDump(CH_SAMPLE_REG(i));
		mvOsRegDump(CH_DBG_REG(i));
		mvOsRegDump(CH_TX_CUR_ADDR_REG(i));
		mvOsRegDump(CH_RX_CUR_ADDR_REG(i));
		mvOsRegDump(CH_ENABLE_REG(i));
		mvOsRegDump(CH_BUFF_OWN_REG(i));
		mvOsRegDump(CH_TX_ADDR_REG(i));
		mvOsRegDump(CH_RX_ADDR_REG(i));
	}
	mvOsPrintf("TDM interrupts:\n");
	mvOsRegDump(INT_EVENT_MASK_REG);
	mvOsRegDump(INT_STATUS_MASK_REG);
	mvOsRegDump(INT_STATUS_REG);  
	for(i=0;i<MV_TDM_MAX_CHANNELS;i++)
	{
		mvOsPrintf("ch%d info:\n",i);
		chInfo = tdmChInfo[i];
		mvOsPrintf("tx ready = %s\n", mvTdmChTxReady((MV_U32)chInfo) ? "yes" : "no");
		mvOsPrintf("rx ready = %s\n", mvTdmChRxReady((MV_U32)chInfo) ? "yes" : "no");
		mvOsPrintf("ex ready = %s\n", mvTdmChExceptionReady((MV_U32)chInfo) ? "yes" : "no");
		mvOsPrintf("RX buffs:\n");
		mvOsPrintf("buff0: virt=%p phys=%p\n",chInfo->rxBuffVirt[0],(MV_U32*)(chInfo->rxBuffPhys[0]));
		mvOsPrintf("buff1: virt=%p phys=%p\n",chInfo->rxBuffVirt[1],(MV_U32*)(chInfo->rxBuffPhys[1]));
		mvOsPrintf("TX buffs:\n");
		mvOsPrintf("buff0: virt=%p phys=%p\n",chInfo->txBuffVirt[0],(MV_U32*)(chInfo->txBuffPhys[0]));
		mvOsPrintf("buff1: virt=%p phys=%p\n",chInfo->txBuffVirt[1],(MV_U32*)(chInfo->txBuffPhys[1]));
	}
}

MV_VOID mvTdmShowProperties(void)
{
	mvOsPrintf("TDM dual channel device rev 0x%x\n", MV_REG_READ(TDM_REV_REG));
}

MV_VOID mvTdmChShowProperties(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_SLIC_INFO *slic = &chInfo->slic;
	mvOsPrintf("SLIC device %s\n", slic->name);
}

MV_U8 currRxSampleGet(MV_U8 ch)
{
	return( MV_REG_BYTE_READ(CH_DBG_REG(ch)+1) );
}
MV_U8 currTxSampleGet(MV_U8 ch)
{
	return( MV_REG_BYTE_READ(CH_DBG_REG(ch)+3) );
}

