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
#include "voiceband/tdm/mvTdm.h"

#ifdef MV_TDM_LINEAR_MODE
#define PCM_SAMPLE_SIZE PCM_SAMPLE_SIZE_2
#else
#define PCM_SAMPLE_SIZE PCM_SAMPLE_SIZE_1
#endif

#define TOTAL_SAMPLE	80
#define INT_SAMPLE	2

/* Helpers */
#define BUFF_IS_FULL	1
#define BUFF_IS_EMPTY	0

static int work_mode;
static int interrupt_mode;


/* TDM structure */
typedef struct _mv_tdm_ch_info
{
	MV_U8 ch;
	MV_U8 *rxBuffVirt[2], *txBuffVirt[2];
	MV_ULONG rxBuffPhys[2], txBuffPhys[2];
	MV_U8 rxBuffFull[2], txBuffFull[2];
	MV_U8 rxCurrBuff, txCurrBuff;
   
} MV_TDM_CH_INFO;

/* Forwards */
static MV_TDM_CH_INFO *tdmChInfo[MV_TDM_MAX_CHANNELS];

static void setDaisyChainMode(void);


static MV_STATUS mvTdmVoiceUnitReset(void);
MV_STATUS mvTdmSpiRead(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs, MV_U8 *data);
MV_STATUS mvTdmSpiWrite(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs);

MV_STATUS mvTdmInit(int workMode, int intMode)
{
	
	MV_TRC_REC("->%s\n",__FUNCTION__);	
	if (mvTdmWinInit() != MV_OK)
	{		
		return MV_ERROR;
	}	
	/* Config TDM */
	MV_REG_BIT_RESET(TDM_SPI_MUX_REG, 1);                       /* enable TDM/SPI interface */
	//if(mvCtrlModelRevGet() == MV_5181L_A1_ID)
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

	/* Reset Voice Unit(FXO/FXS) */
	mvTdmVoiceUnitReset();

	/* set work & interrupt modes */
	work_mode = workMode;
	interrupt_mode = intMode;


	if(work_mode) {
		/* Configure SLIC(s) to work in daisy chain mode */
		MV_TRC_REC("configure daisy chain mode\n");
		setDaisyChainMode();
	}

	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChInit(void *osDev, MV_U8 ch, MV_U32 *tdmCh, MV_VOICE_IF_TYPE type)
{
	MV_TDM_CH_INFO *chInfo;
	MV_U8 buff;
	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,ch);

	if(ch+1 > MV_TDM_MAX_CHANNELS)
	{
		mvOsPrintf("%s: error, support only two voice devices\n",__FUNCTION__);
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

	for(buff=0; buff<2; buff++)
	{
		/* Buffers must be 32B aligned */
		chInfo->rxBuffVirt[buff] = (MV_U8*)mvOsIoUncachedMalloc(NULL, MV_TDM_BUFF_SIZE, &(chInfo->rxBuffPhys[buff]),NULL);
		chInfo->rxBuffFull[buff] = BUFF_IS_EMPTY;

		chInfo->txBuffVirt[buff] = (MV_U8*)mvOsIoUncachedMalloc(NULL, MV_TDM_BUFF_SIZE, &(chInfo->txBuffPhys[buff]),NULL);
		chInfo->txBuffFull[buff] = BUFF_IS_EMPTY;

		if(((MV_ULONG)chInfo->rxBuffVirt[buff] | chInfo->rxBuffPhys[buff] | 
			(MV_ULONG)chInfo->txBuffVirt[buff] | chInfo->txBuffPhys[buff]) & 0x1f) {
			mvOsPrintf("%s: error, unaligned buffer allocation\n", __FUNCTION__);
		}
	}
 
	/* Pass chInfo cookie for upper level */
	*tdmCh = (MV_U32)chInfo;

	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChRemove(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 buff;
	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	mvTdmChStop(tdmCh);

	for(buff=0; buff<2; buff++)
	{
		mvOsIoUncachedFree(NULL, MV_TDM_BUFF_SIZE, chInfo->rxBuffPhys[buff], chInfo->rxBuffVirt[buff],0);
		mvOsIoUncachedFree(NULL, MV_TDM_BUFF_SIZE, chInfo->txBuffPhys[buff], chInfo->txBuffVirt[buff],0);
	}

	mvOsFree(chInfo);

	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

/* Start: only SLIC interrupts are enabled */
MV_STATUS mvTdmChStart(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;

	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	chInfo->rxBuffFull[0] = chInfo->txBuffFull[0] = BUFF_IS_EMPTY;
	chInfo->rxBuffFull[1] = chInfo->txBuffFull[1] = BUFF_IS_EMPTY;

	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

/* Stop: everything is disabled */
MV_STATUS mvTdmChStop(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	
	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	chInfo->rxBuffFull[0] = chInfo->txBuffFull[0] = BUFF_IS_EMPTY;
	chInfo->rxBuffFull[1] = chInfo->txBuffFull[1] = BUFF_IS_EMPTY;

	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

MV_STATUS mvTdmChRxEnable(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);


	chInfo->rxCurrBuff = 0;
	chInfo->rxBuffFull[0] = BUFF_IS_EMPTY;
	chInfo->rxBuffFull[1] = BUFF_IS_EMPTY;
				
	/* Set RX buff */
	MV_REG_WRITE(CH_RX_ADDR_REG(chInfo->ch),chInfo->rxBuffPhys[chInfo->rxCurrBuff]);
	MV_REG_BYTE_WRITE(CH_BUFF_OWN_REG(chInfo->ch)+RX_OWN_BYTE_OFFS, OWN_BY_HW);
	/* Enable RX */
	MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+RX_ENABLE_BYTE_OFFS, CH_ENABLE);
	/* Enable also RX interrupts */		
	MV_REG_WRITE( INT_STATUS_REG, MV_REG_READ(INT_STATUS_REG) & (~(TDM_INT_RX(chInfo->ch))) );
	MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) | TDM_INT_RX(chInfo->ch));


	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}


MV_STATUS mvTdmChTxEnable(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	
	MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) & ~TDM_INT_TX(chInfo->ch)); 
		
	chInfo->txCurrBuff = 0;
	chInfo->txBuffFull[0] = BUFF_IS_EMPTY;
	chInfo->txBuffFull[1] = BUFF_IS_EMPTY;

	/* Set TX buff */
	MV_REG_WRITE(CH_TX_ADDR_REG(chInfo->ch),chInfo->txBuffPhys[chInfo->txCurrBuff]);
	MV_REG_BYTE_WRITE(CH_BUFF_OWN_REG(chInfo->ch)+TX_OWN_BYTE_OFFS, OWN_BY_HW);		
	MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+TX_ENABLE_BYTE_OFFS, CH_ENABLE);		
	/* Enable also TX interrupts */
	MV_REG_WRITE( INT_STATUS_REG, MV_REG_READ(INT_STATUS_REG) & (~(TDM_INT_TX(chInfo->ch))) );
	MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) | TDM_INT_TX(chInfo->ch)); 


	MV_TRC_REC("<-%s\n",__FUNCTION__);	
	return MV_OK;
}


MV_BOOL mvTdmChTxReady(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 nextBuff = !chInfo->txCurrBuff;
	if( (chInfo->txBuffFull[nextBuff] == BUFF_IS_EMPTY))
		return MV_TRUE;
	return MV_FALSE;
}

MV_BOOL mvTdmChRxReady(MV_U32 tdmCh)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 lastBuff = !chInfo->rxCurrBuff;
	if( (chInfo->rxBuffFull[lastBuff] == BUFF_IS_FULL))
		return MV_TRUE;
	return MV_FALSE;
}



/* Determines if any interrupt bit is set*/

/* TX: Copy the data into the next frame buff to be transmitted */
MV_STATUS mvTdmChTx(MV_U32 tdmCh, MV_U8 *osBuff, int count)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 currBuff, nextBuff;
	MV_STATUS ret;

	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);


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
		MV_TRC_REC("app find next tx buff full [irq not ok]\n");
		ret = MV_NOT_READY;
	}


	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return ret; 
}

/* RX: Copy the data from the last recieved buff to application and mark it as empty */
MV_STATUS mvTdmChRx(MV_U32 tdmCh, MV_U8 *osBuff, int count)
{
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;
	MV_U8 currBuff, lastBuff;
	MV_STATUS ret;

	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);


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
		MV_TRC_REC("app find last rx buff empty [irq not ok]\n");
		ret = MV_NOT_READY;
	}


	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return ret;
}



/*
** Called from ISR 
** Mark last frame transmitted buff as empty and give current full buff to HW
** Write procedure mark last frame buff as full after filling it with data
*/
MV_STATUS mvTdmChTxLow(MV_U32 tdmCh)
{
	MV_U8 currBuff, lastBuff;
	MV_U32 max_poll = 0;
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;

	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	/* Change buffers */
	chInfo->txCurrBuff = !chInfo->txCurrBuff;

	currBuff = chInfo->txCurrBuff;
	lastBuff = !currBuff;

	if(chInfo->txBuffFull[currBuff] == BUFF_IS_FULL)
	{
		MV_TRC_REC("curr buff full for hw [app ok]\n");
	}
	else
	{
		MV_TRC_REC("curr buf is empty [app miss write]\n");
	}

	/* Mark last buff that was transmitted by HW as empty. Give it back to HW */
	/* for next frame. The app need to write the data before HW takes it.     */
	memset(chInfo->txBuffVirt[lastBuff], 0, MV_TDM_BUFF_SIZE);
	chInfo->txBuffFull[lastBuff] = BUFF_IS_EMPTY;

	/* Poll on SW ownership (single check) */
	MV_TRC_REC("start poll for SW ownership\n");
	while( ((MV_REG_BYTE_READ(CH_BUFF_OWN_REG(chInfo->ch)+TX_OWN_BYTE_OFFS) & OWNER_MASK) == OWN_BY_HW) 
		&& (max_poll < 2000) )
	{
		mvOsUDelay(1);
		max_poll++;
	}
	if(max_poll == 2000) 
	{
		MV_TRC_REC("poll timeout (~2ms)\n");
		MV_TRC_REC("<-giving up this buff\n");
		return MV_TIMEOUT;
	}
	else
	{
		MV_TRC_REC("tx-low poll stop ok\n");
	}
	
	/*Set TX buff address (must be 32 byte aligned) */ 
	MV_REG_WRITE(CH_TX_ADDR_REG(chInfo->ch),chInfo->txBuffPhys[lastBuff]);

	/* Set HW ownership */
	MV_REG_BYTE_WRITE(CH_BUFF_OWN_REG(chInfo->ch)+TX_OWN_BYTE_OFFS, OWN_BY_HW);

	/* Enable Tx (do I need to do it every time?) */
	MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+TX_ENABLE_BYTE_OFFS, CH_ENABLE);

	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

/* 
** Called from Isr 
** Mark last frame recieved buff as full and give current (empty) buff to HW
** The read procedure will mark the full (last frame) buff as empty after copying the data
*/
MV_STATUS mvTdmChRxLow(MV_U32 tdmCh)
{
	MV_U8 currBuff, lastBuff;
	MV_U32 max_poll = 0;
	MV_TDM_CH_INFO *chInfo = (MV_TDM_CH_INFO *)tdmCh;

	MV_TRC_REC("->%s ch%d\n",__FUNCTION__,chInfo->ch);

	/* Change buffers */
	chInfo->rxCurrBuff = !chInfo->rxCurrBuff;

	currBuff = chInfo->rxCurrBuff;
	lastBuff = !currBuff;

	if(chInfo->rxBuffFull[lastBuff] == BUFF_IS_EMPTY)
	{
		MV_TRC_REC("curr buff empty for hw [app ok]\n");
	}
	else
	{
		MV_TRC_REC("curr buf is full [app miss read]\n");
	}
	
	/* Mark last buff that was received by HW as full. Give it back to HW for */
	/* next frame. The app need to read the data before HW put new data on it.*/
	chInfo->rxBuffFull[lastBuff] = BUFF_IS_FULL;

	/* Poll on SW ownership (single check) */
	MV_TRC_REC("start poll for ownership\n");
	while( ((MV_REG_BYTE_READ(CH_BUFF_OWN_REG(chInfo->ch)+RX_OWN_BYTE_OFFS) & OWNER_MASK) == OWN_BY_HW) 
		&& (max_poll < 2000) )
	{
		mvOsUDelay(1);
		max_poll++;
	}
	if(max_poll == 2000)
	{
		MV_TRC_REC("poll timeout (~2ms)\n");
		MV_TRC_REC("<-giving up this buff\n");
		return MV_TIMEOUT;
	}
	else
	{
		MV_TRC_REC("poll stop ok\n");
	}
	
	/* Set RX buff address (must be 32 byte aligned) */
	MV_REG_WRITE(CH_RX_ADDR_REG(chInfo->ch),chInfo->rxBuffPhys[lastBuff]);

	/* Set HW ownership */
	MV_REG_BYTE_WRITE(CH_BUFF_OWN_REG(chInfo->ch)+RX_OWN_BYTE_OFFS, OWN_BY_HW);		
	
	/* Enable Rx (do I need to do it every time?) */
	MV_REG_BYTE_WRITE(CH_ENABLE_REG(chInfo->ch)+RX_ENABLE_BYTE_OFFS, CH_ENABLE);

	MV_TRC_REC("<-%s\n",__FUNCTION__);
	return MV_OK;
}

/*******************************
**        SLIC Helpers        **
*******************************/

static MV_STATUS mvTdmVoiceUnitReset(void)
{
	MV_TRC_REC("reseting voice unit(s)\n");
	MV_REG_WRITE(MISC_CTRL_REG,0);
	mvOsDelay(250);
	MV_REG_WRITE(MISC_CTRL_REG,1);
	mvOsDelay(250);
	return MV_OK;
}

/****************************
**        SPI Stuff        **
****************************/

  static MV_U8 currentCS = 0xFF;


MV_VOID mvTdmSetCurrentUnit(MV_32 cs)
{
//	MV_TRC_REC("%s: slic cs %d\n",__FUNCTION__,slic->cs);
	if(!work_mode) {
		if(cs == 0)
		{
	  		MV_REG_WRITE(PCM_CTRL_REG, (MV_REG_READ(PCM_CTRL_REG) & ~CS_CTRL));
		}
		else
		{
	  		MV_REG_WRITE(PCM_CTRL_REG, (MV_REG_READ(PCM_CTRL_REG) | CS_CTRL));
		}
	}
	else
		currentCS = cs;

			
}


static void setDaisyChainMode(void)
{
	mvOsPrintf("Setting Daisy Chain Mode\n");
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);
	MV_REG_WRITE(SPI_CODEC_CMD_LO_REG, (0x80<<8) | 0);
	MV_REG_WRITE(SPI_CODEC_CTRL_REG, TRANSFER_BYTES(2) | ENDIANESS_MSB_MODE | WR_MODE | CLK_SPEED_LO_DIV);
	MV_REG_WRITE(SPI_CTRL_REG, MV_REG_READ(SPI_CTRL_REG) | SPI_ACTIVE);
	mvOsDelay(50);
}


MV_STATUS mvTdmSpiWrite(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs)
{

	
	/*MV_TRC_REC("%s: cs = %d val1 = 0x%x val2 = 0x%x\n",__FUNCTION__,cs, val1, val2);*/


	/* Poll for ready indication */
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);

	if(!work_mode) {
		if(cs == 0)
		{
	  		MV_REG_WRITE(PCM_CTRL_REG, (MV_REG_READ(PCM_CTRL_REG) & ~CS_CTRL));
		}
		else
		{
	  		MV_REG_WRITE(PCM_CTRL_REG, (MV_REG_READ(PCM_CTRL_REG) | CS_CTRL));
		}
	}
	
	MV_REG_WRITE(SPI_CODEC_CMD_LO_REG, val1);
	MV_REG_WRITE(SPI_CODEC_CMD_HI_REG, val2);
	MV_REG_WRITE(SPI_CODEC_CTRL_REG, cmd);

	/* Activate */
	MV_REG_WRITE(SPI_CTRL_REG, MV_REG_READ(SPI_CTRL_REG) | SPI_ACTIVE);

	/* Poll for ready indication */
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);

	return MV_OK;
}

MV_STATUS mvTdmSpiRead(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs, MV_U8 *data)
{
		
	/*MV_TRC_REC("%s: cs = %d val1 = 0x%x val2 = 0x%x\n",__FUNCTION__,cs, val1, val2);*/

	/* Poll for ready indication */
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);

	if(!work_mode) {
		if(cs == 0)
		{
	  		MV_REG_WRITE(PCM_CTRL_REG, (MV_REG_READ(PCM_CTRL_REG) & ~CS_CTRL));
		}
		else
		{
	  		MV_REG_WRITE(PCM_CTRL_REG, (MV_REG_READ(PCM_CTRL_REG) | CS_CTRL));
		}
	}

	MV_REG_WRITE(SPI_CODEC_CMD_LO_REG, val1);
	MV_REG_WRITE(SPI_CODEC_CMD_HI_REG, val2);
	MV_REG_WRITE(SPI_CODEC_CTRL_REG, cmd);

	/* Activate */
	MV_REG_WRITE(SPI_CTRL_REG, MV_REG_READ(SPI_CTRL_REG) | SPI_ACTIVE);

	/* Poll for ready indication */
	while( (MV_REG_READ(SPI_CTRL_REG) & SPI_STAT_MASK) == SPI_ACTIVE);

	*data = MV_REG_BYTE_READ(SPI_CODEC_READ_DATA_REG);

//	MV_TRC_REC(" data=0x%x\n",*data);
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


MV_U8 currRxSampleGet(MV_U8 ch)
{
	return( MV_REG_BYTE_READ(CH_DBG_REG(ch)+1) );
}
MV_U8 currTxSampleGet(MV_U8 ch)
{
	return( MV_REG_BYTE_READ(CH_DBG_REG(ch)+3) );
}




