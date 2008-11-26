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
#ifndef __INCmvTdmh
#define __INCmvTdmh

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/mvCtrlEnvAddrDec.h"
#include "voiceband/tdm/mvTdmRegs.h"

#define MV_TDM_MAX_CHANNELS 2

#define MV_TDM_READ_INTR		0x1
#define MV_TDM_WRITE_INTR		0x2
#define MV_TDM_SLIC_INTR		0x4
#define MV_TDM_DMA_ABORT_INTR		0x80000000
#define MV_CHANNEL_INTR(ch)		((ch) * 4)

/* TDM configuration */
#define CH0_RX_SLOT	0
#define CH0_TX_SLOT	0
#define CH1_RX_SLOT	10
#define CH1_TX_SLOT	10


#if defined(MV_TDM_LINEAR_MODE)
 #define MV_TDM_BUFF_SIZE 160
#elif defined(MV_TDM_ULAW_MODE)
 #define MV_TDM_BUFF_SIZE 80
#else
 #define MV_TDM_BUFF_SIZE 160
 #warning "Default sampling mode set to Linear"
#endif



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


typedef struct _mvTdmDecWin
{
        MV_TARGET     target;
        MV_ADDR_WIN   addrWin; /* An address window*/
        MV_BOOL       enable;  /* Address decode window is enabled/disabled */ 
}MV_TDM_DEC_WIN;

typedef enum _voice_if_type
{
  MV_FXO = 0,
  MV_FXS,
}MV_VOICE_IF_TYPE;


MV_STATUS mvTdmInit(int workMode, int intMode);
MV_STATUS mvTdmChInit(void *osDev, MV_U8 ch, MV_U32 *tdmCh, MV_VOICE_IF_TYPE type);
MV_STATUS mvTdmChRemove(MV_U32 tdmCh);
MV_STATUS mvTdmChStart(MV_U32 tdmCh);
MV_STATUS mvTdmChStop(MV_U32 tdmCh);
MV_STATUS mvTdmChRxEnable(MV_U32 tdmCh);
MV_STATUS mvTdmChTxEnable(MV_U32 tdmCh);
MV_STATUS mvTdmChTx(MV_U32 tdmCh, MV_U8 *buff, int count);
MV_STATUS mvTdmChRx(MV_U32 tdmCh, MV_U8 *buff, int count);
MV_BOOL mvTdmChTxReady(MV_U32 tdmCh);
MV_BOOL mvTdmChRxReady(MV_U32 tdmCh);
MV_STATUS mvTdmChTxLow(MV_U32 tdmCh);
MV_STATUS mvTdmChRxLow(MV_U32 tdmCh);
MV_VOID mvTdmShowProperties(void);
MV_VOID mvTdmRegsDump(void);


MV_STATUS mvTdmWinInit(MV_VOID);
MV_STATUS mvTdmWinSet(MV_U32 winNum, MV_TDM_DEC_WIN *pAddrDecWin);
MV_STATUS mvTdmWinGet(MV_U32 winNum, MV_TDM_DEC_WIN *pAddrDecWin);
MV_STATUS mvTdmWinEnable(int winNum, MV_BOOL enable);
MV_VOID mvTdmAddrDecShow(MV_VOID);


#endif /* __INCmvTdmh */
