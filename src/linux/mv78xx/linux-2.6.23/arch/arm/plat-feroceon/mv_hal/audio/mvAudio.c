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
#include "mvAudio.h"
#include "ctrlEnv/sys/mvSysAudio.h"

static MV_U32 audioBurstBytesNumGet(MV_AUDIO_BURST_SIZE burst);

/*******************************************************************************
* mvAudioHalInit - Initialize the Audio subsystem
*
* DESCRIPTION:
*
* INPUT:
*       None
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvAudioHalInit(void)
{
    int timeout;

    MV_REG_BIT_RESET(AUDIO_REG_BASE + 0x1200,0x333FF8);
    MV_REG_BIT_SET(AUDIO_REG_BASE + 0x1200,0x111D18);

    /*MV_REG_BIT_RESET(0x10074,0xC018000);
    MV_REG_BIT_SET(0x10074,0x4008000);*/

    timeout = 10000000;
    while(timeout--);

    MV_REG_BIT_RESET(AUDIO_REG_BASE + 0x1200,0x333FF8);
    MV_REG_BIT_SET(AUDIO_REG_BASE + 0x1200,0x111D18);

    /*MV_REG_BIT_RESET(0x10074,0xC018000);
    MV_REG_BIT_SET(0x10074,0x4008000);*/

}



/* Clocks Control and Status related*/
/*******************************************************************************
* mvAudioDCOCtrlSet - Set DCO control register
*
* DESCRIPTION:
*
* INPUT:
*       dcoCtrl: pointer to MV_AUDIO_FREQ_DATA structure
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/

MV_STATUS mvAudioDCOCtrlSet(MV_AUDIO_FREQ_DATA *dcoCtrl)
{
	MV_U32 reg;
	/* Check parameters*/
	if (dcoCtrl->baseFreq > AUDIO_FREQ_96KH)
	{
		mvOsPrintf("mvAudioDCOCtrlSet: dcoCtrl->baseFreq value (0x%x) invalid\n",
				   dcoCtrl->baseFreq);
		return MV_BAD_PARAM;

	}
	if ((dcoCtrl->offset > 0xFD0)||(dcoCtrl->offset < 0x20))
	{
		mvOsPrintf("mvAudioDCOCtrlSet: dcoCtrl->offset value (0x%x) invalid\n",
				   dcoCtrl->baseFreq);
		return MV_BAD_PARAM;
	}
	reg = MV_REG_READ(MV_AUDIO_DCO_CTRL_REG);

	reg &= ~(ADCR_DCO_CTRL_FS_MASK|ADCR_DCO_CTRL_OFFSET_MASK);
	reg |= ((dcoCtrl->baseFreq << ADCR_DCO_CTRL_FS_OFFS)  |
			(dcoCtrl->offset << ADCR_DCO_CTRL_OFFSET_OFFS));
	MV_REG_WRITE(MV_AUDIO_DCO_CTRL_REG, reg);

	return MV_OK;
}

/*******************************************************************************
* mvAudioDCOCtrlGet - Set DCO control register
*
* DESCRIPTION:
*
* INPUT:
*       dcoCtrl: pointer to MV_AUDIO_FREQ_DATA structure
* OUTPUT:
*		dcoCtrl: pointer to MV_AUDIO_FREQ_DATA structure
* RETURN:
*       None
*
*******************************************************************************/

MV_VOID mvAudioDCOCtrlGet(MV_AUDIO_FREQ_DATA *dcoCtrl)
{
	MV_U32 reg = MV_REG_READ(MV_AUDIO_DCO_CTRL_REG);

	dcoCtrl->baseFreq = (reg & ADCR_DCO_CTRL_FS_MASK) >>  ADCR_DCO_CTRL_FS_OFFS ;
	dcoCtrl->offset = (reg & ADCR_DCO_CTRL_OFFSET_MASK) >>  ADCR_DCO_CTRL_OFFSET_OFFS ;
}
/*******************************************************************************
* mvAudioSpcrCtrlGet - Set SPCR control register
*
* DESCRIPTION:
*
* INPUT:
*       spcrCtrl: pointer to MV_AUDIO_FREQ_DATA structure
* OUTPUT:
*		spcrCtrl: pointer to MV_AUDIO_FREQ_DATA structure
* RETURN:
*       None
*
*******************************************************************************/

MV_VOID mvAudioSpcrCtrlGet(MV_AUDIO_FREQ_DATA *spcrCtrl)
{
	MV_U32 reg = MV_REG_READ(MV_AUDIO_SPCR_DCO_STATUS_REG);

	spcrCtrl->baseFreq = (reg & ASDSR_SPCR_CTRLFS_MASK) >>  ASDSR_SPCR_CTRLFS_OFFS ;
	spcrCtrl->offset = (reg & ASDSR_SPCR_CTRLOFFSET_MASK) >>  ASDSR_SPCR_CTRLOFFSET_OFFS;
}


/* Audio PlayBack related*/
/*******************************************************************************
* mvAudioPlaybackControlSet - Set Playback general parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_AUDIO_PLAYBACK_CTRL structure
* OUTPUT:
*		None
* RETURN:
*       MV_OK on success , MV_FAIL on fail
*
*******************************************************************************/
MV_STATUS mvAudioPlaybackControlSet(MV_AUDIO_PLAYBACK_CTRL *ctrl)
{
	MV_AUDIO_DEC_WIN audioWin;
	MV_CPU_DEC_WIN  cpuWin;
	MV_ADDR_WIN   bufAddrWin;
	MV_U32 target;
	MV_U32 reg;

	if (ctrl->monoMode >= AUDIO_PLAY_OTHER_MONO)
	{
		mvOsPrintf("mvAudioPlaybackControlSet: Error ,illegal monoMode %x\n",
				   ctrl->monoMode );

		return MV_FAIL;

	}

	if ((ctrl->burst != AUDIO_32BYTE_BURST) &&
		(ctrl->burst != AUDIO_128BYTE_BURST))
	{
		mvOsPrintf("mvAudioPlaybackControlSet: Error ,illegal burst %x\n",
				   ctrl->burst );

		return MV_FAIL;

	}

	if (ctrl->bufferPhyBase & (MV_AUDIO_BUFFER_MIN_ALIGN - 1))
	{
		mvOsPrintf("mvAudioPlaybackControlSet: Error ,bufferPhyBase is not"\
				   "\n aligned to 0x%x bytes\n",MV_AUDIO_BUFFER_MIN_ALIGN );

		return MV_FAIL;
	}

	if  ((ctrl->bufferSize <= audioBurstBytesNumGet(ctrl->burst))||
		 (ctrl->bufferSize & (audioBurstBytesNumGet(ctrl->burst) - 1))||
		 (ctrl->bufferSize > AUDIO_REG_TO_SIZE(APBBCR_SIZE_MAX))
		 )
	{
		mvOsPrintf("mvAudioPlaybackControlSet: Error, bufferSize smaller"\
				   "\nthan or not multiple of 0x%x bytes or larger than"\
				   "\n 0x%x",
				   audioBurstBytesNumGet(ctrl->burst),
				   AUDIO_REG_TO_SIZE(APBBCR_SIZE_MAX));

		return MV_FAIL;
	}


	reg = MV_REG_READ(MV_AUDIO_PLAYBACK_CTRL_REG);
	reg &= ~(APCR_PLAY_BURST_SIZE_MASK|APCR_LOOPBACK_MASK|APCR_PLAY_MONO_MASK |
             APCR_PLAY_SAMPLE_SIZE_MASK);
	reg |= ctrl->burst << APCR_PLAY_BURST_SIZE_OFFS;
	reg |= ctrl->loopBack << APCR_LOOPBACK_OFFS;
	reg |= ctrl->monoMode << APCR_PLAY_MONO_OFFS;
    reg |= ctrl->sampleSize << APCR_PLAY_SAMPLE_SIZE_OFFS;
	MV_REG_WRITE(MV_AUDIO_PLAYBACK_CTRL_REG, reg);

	/* Get the details of the Playback address window*/
	if( mvAudioWinGet( MV_AUDIO_PLAYBACK_WIN_NUM, &audioWin ) != MV_OK )
	{
		mvOsPrintf("mvAudioPlaybackControlSet: Error calling mvAudioWinGet on win %d\n",
				   MV_AUDIO_PLAYBACK_WIN_NUM);
		return MV_FAIL;
	}

	bufAddrWin.baseHigh = 0;
	bufAddrWin.baseLow = ctrl->bufferPhyBase;
	bufAddrWin.size = ctrl->bufferSize;

	/* If Playback window is not enabled or buffer address is not within window boundries
	   then try to set a new value to the Playback window by
	   Geting the target of where the buffer exist, if the buffer is within the window
	   of the new target then set the Playback window to that target
	   else return Fail
    */

	if((audioWin.enable != MV_TRUE) ||
	  (MV_TRUE != ctrlWinWithinWinTest(&bufAddrWin, &audioWin.addrWin)))
	{
		/* Get the target of the buffer that user require*/
		target = mvCpuIfTargetOfBaseAddressGet(ctrl->bufferPhyBase);
		if (MAX_TARGETS == target)
		{
			mvOsPrintf("mvCpuIfTargetOfBaseAddressGet: Error calling mvAudioWinGet on address 0x%x\n",
					   ctrl->bufferPhyBase);
			return MV_FAIL;
		}

		/* Get the window details of this target*/
		if (MV_OK != mvCpuIfTargetWinGet(target, &cpuWin))
		{
			mvOsPrintf("mvAudioPlaybackControlSet: Error calling mvCpuIfTargetWinGet on target %d\n",
					   target);
			return MV_FAIL;

		}

		/* if the address window of the target is enabled and te user buffer is within
		   that target address window then set the palyback\recording window to the
		   target window

		*/
		if((cpuWin.enable == MV_TRUE) &&
		  (MV_TRUE == ctrlWinWithinWinTest(&bufAddrWin, &cpuWin.addrWin)))
		{
			audioWin.addrWin.baseHigh = cpuWin.addrWin.baseHigh;
			audioWin.addrWin.baseLow = cpuWin.addrWin.baseLow;
			audioWin.addrWin.size = cpuWin.addrWin.size;
			audioWin.enable = cpuWin.enable;
			audioWin.target = target;


			if( mvAudioWinSet( MV_AUDIO_PLAYBACK_WIN_NUM, &audioWin ) != MV_OK )
			{
				mvOsPrintf("mvAudioPlaybackControlSet: Error calling mvAudioWinGet on win %d\n",
						   MV_AUDIO_PLAYBACK_WIN_NUM);
				return MV_FAIL;
			}

		}
		else
		{
			mvOsPrintf("mvAudioPlaybackControlSet: Error buffer is not within a valid target\n");
			return MV_FAIL;

		}
	}

    /* Set the interrupt byte count.                            */
    reg = ctrl->intByteCount & APBCI_BYTE_COUNT_MASK;
    MV_REG_WRITE(MV_AUDIO_PLAYBACK_BYTE_CNTR_INT_REG, reg);


	MV_REG_WRITE(MV_AUDIO_PLAYBACK_BUFF_START_REG, ctrl->bufferPhyBase);
	MV_REG_WRITE(MV_AUDIO_PLAYBACK_BUFF_SIZE_REG,
				 AUDIO_SIZE_TO_REG(ctrl->bufferSize));

	return MV_OK;
}

/*******************************************************************************
* mvAudioPlaybackControlGet - Get Playback general parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_AUDIO_PLAYBACK_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_AUDIO_PLAYBACK_CTRL structure
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvAudioPlaybackControlGet(MV_AUDIO_PLAYBACK_CTRL *ctrl)
{

	MV_U32 reg = MV_REG_READ(MV_AUDIO_PLAYBACK_CTRL_REG);

	ctrl->burst = (reg & APCR_PLAY_BURST_SIZE_MASK) >> APCR_PLAY_BURST_SIZE_OFFS;
	ctrl->loopBack = (reg & APCR_LOOPBACK_MASK) >> APCR_LOOPBACK_OFFS;
	ctrl->monoMode = (reg & APCR_PLAY_MONO_MASK) >> APCR_PLAY_MONO_OFFS;

	ctrl->bufferPhyBase = MV_REG_READ(MV_AUDIO_PLAYBACK_BUFF_START_REG);
	reg = MV_REG_READ(MV_AUDIO_PLAYBACK_BUFF_SIZE_REG);
	ctrl->bufferSize = AUDIO_REG_TO_SIZE(reg);

    ctrl->intByteCount = MV_REG_READ(MV_AUDIO_PLAYBACK_BYTE_CNTR_INT_REG);

}
/*******************************************************************************
* mvAudioPlaybackStatusGet - Get Playback status parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_AUDIO_PLAYBACK_STATUS structure
* OUTPUT:
*		ctrl: pointer to MV_AUDIO_PLAYBACK_STATUS structure
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvAudioPlaybackStatusGet(MV_AUDIO_PLAYBACK_STATUS *status)
{

	status->muteI2S = ((MV_REG_READ(MV_AUDIO_PLAYBACK_CTRL_REG)&APCR_PLAY_I2S_MUTE_MASK)?
						MV_TRUE:MV_FALSE);
	status->enableI2S = ((MV_REG_READ(MV_AUDIO_PLAYBACK_CTRL_REG)&APCR_PLAY_I2S_ENABLE_MASK)?
						MV_TRUE:MV_FALSE);
	status->muteSPDIF = ((MV_REG_READ(MV_AUDIO_PLAYBACK_CTRL_REG)&APCR_PLAY_SPDIF_MUTE_MASK)?
						MV_TRUE:MV_FALSE);
	status->enableSPDIF = ((MV_REG_READ(MV_AUDIO_PLAYBACK_CTRL_REG)&APCR_PLAY_SPDIF_ENABLE_MASK)?
						MV_TRUE:MV_FALSE);
	status->pause = ((MV_REG_READ(MV_AUDIO_PLAYBACK_CTRL_REG)&APCR_PLAY_PAUSE_MASK)?
						MV_TRUE:MV_FALSE);

}

/*******************************************************************************
* mvSPDIFPlaybackCtrlGet - Set SPDIF Playback control parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_SPDIF_PLAYBACK_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_SPDIF_PLAYBACK_CTRL structure
* RETURN:
*       None
*
*******************************************************************************/
/* Audio SPDIF PlayBack related*/
MV_VOID mvSPDIFPlaybackCtrlSet(MV_SPDIF_PLAYBACK_CTRL *ctrl)
{
	ctrl->blockStartInternally?
		(MV_REG_BIT_SET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_BLOCK_START_MASK)):
		(MV_REG_BIT_RESET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_BLOCK_START_MASK));

	ctrl->validityFromMemory?
		(MV_REG_BIT_SET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_PB_EN_MEM_VALIDITY_MASK)):
		(MV_REG_BIT_RESET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_PB_EN_MEM_VALIDITY_MASK));

	ctrl->userBitsFromMemory?
		(MV_REG_BIT_SET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_PB_MEM_USR_EN_MASK)):
		(MV_REG_BIT_RESET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_PB_MEM_USR_EN_MASK));

	ctrl->underrunData?
		(MV_REG_BIT_SET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_UNDERRUN_DATA_MASK)):
		(MV_REG_BIT_RESET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_UNDERRUN_DATA_MASK));

	ctrl->validity?
		(MV_REG_BIT_SET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_PB_REG_VALIDITY_MASK)):
		(MV_REG_BIT_RESET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_PB_REG_VALIDITY_MASK));

	ctrl->nonPcm?
		(MV_REG_BIT_SET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_PB_NONPCM_MASK)):
		(MV_REG_BIT_RESET(MV_AUDIO_SPDIF_PLAY_CTRL_REG,ASPCR_SPDIF_PB_NONPCM_MASK));



}
/*******************************************************************************
* mvSPDIFPlaybackCtrlGet - Get SPDIF Playback control parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_SPDIF_PLAYBACK_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_SPDIF_PLAYBACK_CTRL structure
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvSPDIFPlaybackCtrlGet(MV_SPDIF_PLAYBACK_CTRL *ctrl)
{
	ctrl->blockStartInternally =
			((MV_REG_READ(MV_AUDIO_SPDIF_PLAY_CTRL_REG)&ASPCR_SPDIF_BLOCK_START_MASK)?
			 MV_TRUE:MV_FALSE);
	ctrl->nonPcm =
			((MV_REG_READ(MV_AUDIO_SPDIF_PLAY_CTRL_REG)&ASPCR_SPDIF_PB_NONPCM_MASK)?
			 MV_TRUE:MV_FALSE);
	ctrl->underrunData =
			((MV_REG_READ(MV_AUDIO_SPDIF_PLAY_CTRL_REG)&ASPCR_SPDIF_UNDERRUN_DATA_MASK)?
			 MV_TRUE:MV_FALSE);
	ctrl->userBitsFromMemory =
			((MV_REG_READ(MV_AUDIO_SPDIF_PLAY_CTRL_REG)&ASPCR_SPDIF_PB_MEM_USR_EN_MASK)?
			 MV_TRUE:MV_FALSE);
	ctrl->validity =
			((MV_REG_READ(MV_AUDIO_SPDIF_PLAY_CTRL_REG)&ASPCR_SPDIF_PB_REG_VALIDITY_MASK)?
			 MV_TRUE:MV_FALSE);
	ctrl->validityFromMemory =
			((MV_REG_READ(MV_AUDIO_SPDIF_PLAY_CTRL_REG)&ASPCR_SPDIF_PB_EN_MEM_VALIDITY_MASK)?
			 MV_TRUE:MV_FALSE);



}

/*******************************************************************************
* mvI2SPlaybackCtrlSet - Set I2S Playback control parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_I2S_PLAYBACK_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_I2S_PLAYBACK_CTRL structure
* RETURN:
*       MV_OK on success, and MV_FAIL on fail.
*
*******************************************************************************/

/* Audio I2S PlayBack related*/
MV_STATUS mvI2SPlaybackCtrlSet(MV_I2S_PLAYBACK_CTRL *ctrl)
{
	MV_U32 reg = MV_REG_READ(MV_AUDIO_I2S_PLAY_CTRL_REG) &
				~(AIPCR_I2S_PB_JUSTF_MASK|AIPCR_I2S_PB_SAMPLE_SIZE_MASK);

	if (ctrl->sampleSize > SAMPLE_16BIT)
	{
		mvOsPrintf("mvI2SPlaybackCtrlSet: illigal sample size\n");
		return MV_FAIL;
	}

	reg |= ctrl->sampleSize << AIPCR_I2S_PB_SAMPLE_SIZE_OFFS;

	if (ctrl->sendLastFrame)
	{
		MV_REG_BIT_SET(MV_AUDIO_I2S_PLAY_CTRL_REG,AIPCR_I2S_SEND_LAST_FRM_MASK);
	}
	else
	{
		MV_REG_BIT_RESET(MV_AUDIO_I2S_PLAY_CTRL_REG,AIPCR_I2S_SEND_LAST_FRM_MASK);
	}


	switch (ctrl->justification)
	{
	case I2S_JUSTIFIED:
	case LEFT_JUSTIFIED:
	case RIGHT_JUSTIFIED:
		reg |= ctrl->justification << AIPCR_I2S_PB_JUSTF_OFFS;
		break;
	default:
		mvOsPrintf("mvI2SPlaybackCtrlSet: illigal Justification value\n");
		return MV_FAIL;

	}

	MV_REG_WRITE(MV_AUDIO_I2S_PLAY_CTRL_REG, reg);
	return MV_OK;

}

/*******************************************************************************
* mvI2SPlaybackCtrlGet - Get I2S Playback control parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_I2S_PLAYBACK_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_I2S_PLAYBACK_CTRL structure
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvI2SPlaybackCtrlGet(MV_I2S_PLAYBACK_CTRL *ctrl)
{
	ctrl->sendLastFrame =
			((MV_REG_READ(MV_AUDIO_I2S_PLAY_CTRL_REG)&AIPCR_I2S_SEND_LAST_FRM_MASK)?
			 MV_TRUE:MV_FALSE);

	ctrl->justification =
			((MV_REG_READ(MV_AUDIO_I2S_PLAY_CTRL_REG)&AIPCR_I2S_PB_JUSTF_MASK) >>
			 AIPCR_I2S_PB_JUSTF_OFFS);

	ctrl->sampleSize =
		((MV_REG_READ(MV_AUDIO_I2S_PLAY_CTRL_REG)&AIPCR_I2S_PB_SAMPLE_SIZE_MASK ) >>
		 AIPCR_I2S_PB_SAMPLE_SIZE_OFFS);

}
/*******************************************************************************
* mvAudioRecordControlGet - Get Recording control parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_AUDIO_RECORD_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_AUDIO_RECORD_CTRL structure
* RETURN:
*       MV_OK on success , MV_FAIL on fail.
*
*******************************************************************************/
/* Audio Recording*/
MV_STATUS mvAudioRecordControlSet(MV_AUDIO_RECORD_CTRL *ctrl)
{
	MV_AUDIO_DEC_WIN audioWin;
	MV_CPU_DEC_WIN  cpuWin;
	MV_ADDR_WIN   bufAddrWin;
	MV_U32 target;
	MV_U32 reg;

	if (ctrl->monoChannel > AUDIO_REC_RIGHT_MONO)
	{
		mvOsPrintf("mvAudioRecordControlSet: Error ,illegal monoChannel %x\n",
				   ctrl->monoChannel );

		return MV_FAIL;
	}

	if ((ctrl->burst != AUDIO_32BYTE_BURST) &&
		(ctrl->burst != AUDIO_128BYTE_BURST))
	{
		mvOsPrintf("mvAudioRecordControlSet: Error ,illegal burst %x\n",
				   ctrl->burst );

		return MV_FAIL;

	}

	if (ctrl->bufferPhyBase & (MV_AUDIO_BUFFER_MIN_ALIGN - 1))
	{
		mvOsPrintf("mvAudioRecordControlSet: Error ,bufferPhyBase is not"\
				   "\n aligned to 0x%x bytes\n",MV_AUDIO_BUFFER_MIN_ALIGN );

		return MV_FAIL;
	}

	if  ((ctrl->bufferSize <= audioBurstBytesNumGet(ctrl->burst))||
		(ctrl->bufferSize & (audioBurstBytesNumGet(ctrl->burst) - 1))||
		 (ctrl->bufferSize > AUDIO_REG_TO_SIZE(APBBCR_SIZE_MAX))
		 )
	{
		mvOsPrintf("mvAudioRecordControlSet: Error, bufferSize smaller"\
				   "\nthan or not multiple of 0x%x bytes or larger than"\
				   "\n 0x%x",
				   audioBurstBytesNumGet(ctrl->burst),
				   AUDIO_REG_TO_SIZE(APBBCR_SIZE_MAX));

		return MV_FAIL;
	}


	reg = MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG);
	reg &= ~(ARCR_RECORD_BURST_SIZE_MASK|ARCR_RECORDED_MONO_CHNL_MASK|
			 ARCR_RECORD_SAMPLE_SIZE_MASK);
	switch (ctrl->sampleSize)
	{
	case SAMPLE_16BIT:
	case SAMPLE_16BIT_NON_COMPACT:
	case SAMPLE_20BIT:
	case SAMPLE_24BIT:
	case SAMPLE_32BIT:
		reg |= ctrl->sampleSize << ARCR_RECORD_SAMPLE_SIZE_OFFS;
        break;
	default:
        mvOsPrintf("mvAudioRecordControlSet: Error ,illegal sampleSize %x\n",
				   ctrl->sampleSize );

		return MV_FAIL;
	}

	reg |= ctrl->burst << ARCR_RECORD_BURST_SIZE_OFFS;
	reg |= ctrl->monoChannel << ARCR_RECORDED_MONO_CHNL_OFFS;
	MV_REG_WRITE(MV_AUDIO_RECORD_CTRL_REG, reg);

	if (ctrl->mono)
		MV_REG_BIT_SET (MV_AUDIO_RECORD_CTRL_REG, 
				ARCR_RECORD_MONO_MASK);
	else
		MV_REG_BIT_RESET (MV_AUDIO_RECORD_CTRL_REG, 
				ARCR_RECORD_MONO_MASK);
		
	/* Get the details of the Record address window*/
	if( mvAudioWinGet( MV_AUDIO_RECORD_WIN_NUM, &audioWin ) != MV_OK )
	{
		mvOsPrintf("mvAudioRecordControlSet: Error calling mvAudioWinGet on win %d\n",
				   MV_AUDIO_RECORD_WIN_NUM);
		return MV_FAIL;
	}

	bufAddrWin.baseHigh = 0;
	bufAddrWin.baseLow = ctrl->bufferPhyBase;
	bufAddrWin.size = ctrl->bufferSize;

	/* If Record window is not enabled or buffer address is not within window boundries
	   then try to set a new value to the Record window by
	   Geting the target of where the buffer exist, if the buffer is within the window
	   of the new target then set the Record window to that target
	   else return Fail
    */

	if((audioWin.enable != MV_TRUE) ||
	  (MV_TRUE != ctrlWinWithinWinTest(&bufAddrWin, &audioWin.addrWin)))
	{
		/* Get the target of the buffer that user require*/
		target = mvCpuIfTargetOfBaseAddressGet(ctrl->bufferPhyBase);
		if (MAX_TARGETS == target)
		{
			mvOsPrintf("mvAudioRecordControlSet: Error calling mvAudioWinGet on address 0x%x\n",
					   ctrl->bufferPhyBase);
			return MV_FAIL;
		}

		/* Get the window details of this target*/
		if (MV_OK != mvCpuIfTargetWinGet(target, &cpuWin))
		{
			mvOsPrintf("mvAudioRecordControlSet: Error calling mvCpuIfTargetWinGet on target %d\n",
					   target);
			return MV_FAIL;

		}

		/* if the address window of the target is enabled and te user buffer is within
		   that target address window then set the palyback\recording window to the
		   target window

		*/
		if((cpuWin.enable == MV_TRUE) &&
		  (MV_TRUE == ctrlWinWithinWinTest(&bufAddrWin, &cpuWin.addrWin)))
		{

			audioWin.addrWin.baseHigh = cpuWin.addrWin.baseHigh;
			audioWin.addrWin.baseLow = cpuWin.addrWin.baseLow;
			audioWin.addrWin.size = cpuWin.addrWin.size;
			audioWin.enable = cpuWin.enable;
			audioWin.target = target;

			if( mvAudioWinSet( MV_AUDIO_RECORD_WIN_NUM, &audioWin ) != MV_OK )
			{
				mvOsPrintf("mvAudioRecordControlSet: Error calling mvAudioWinGet on win %d\n",
						   MV_AUDIO_RECORD_WIN_NUM);
				return MV_FAIL;
			}

		}
		else
		{
			mvOsPrintf("mvAudioRecordControlSet: Error buffer is not within a valid target\n");
			return MV_FAIL;

		}
	}

    /* Set the interrupt byte count.                            */
    reg = ctrl->intByteCount & ARBCI_BYTE_COUNT_MASK;
    MV_REG_WRITE(MV_AUDIO_RECORD_BYTE_CNTR_INT_REG, reg);


	MV_REG_WRITE(MV_AUDIO_RECORD_START_ADDR_REG, ctrl->bufferPhyBase);
	MV_REG_WRITE(MV_AUDIO_RECORD_BUFF_SIZE_REG,
				 AUDIO_SIZE_TO_REG(ctrl->bufferSize));

	return MV_OK;
}

/*******************************************************************************
* mvAudioRecordControlGet - Get Recording control parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_AUDIO_RECORD_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_AUDIO_RECORD_CTRL structure
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvAudioRecordControlGet(MV_AUDIO_RECORD_CTRL *ctrl)
{
	MV_U32 reg;

	ctrl->mono =
			((MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG)&ARCR_RECORD_MONO_MASK)?
			 MV_TRUE:MV_FALSE);

	ctrl->burst =
			((MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG)&ARCR_RECORD_BURST_SIZE_MASK) >>
			 ARCR_RECORD_BURST_SIZE_OFFS);

	ctrl->monoChannel =
			((MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG)&ARCR_RECORDED_MONO_CHNL_MASK) >>
			 ARCR_RECORDED_MONO_CHNL_OFFS);

	ctrl->sampleSize =
			((MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG)&ARCR_RECORD_SAMPLE_SIZE_MASK) >>
			 ARCR_RECORD_SAMPLE_SIZE_OFFS);

	ctrl->bufferPhyBase = MV_REG_READ(MV_AUDIO_RECORD_START_ADDR_REG);
	reg = MV_REG_READ(MV_AUDIO_RECORD_BUFF_SIZE_REG);
	ctrl->bufferSize = AUDIO_REG_TO_SIZE(reg);

    ctrl->intByteCount = MV_REG_READ(MV_AUDIO_RECORD_BYTE_CNTR_INT_REG);

}

/*******************************************************************************
* mvAudioRecordControlGet - Get Recording status parameters
*
* DESCRIPTION:
*
* INPUT:
*       status: pointer to MV_AUDIO_RECORD_STATUS structure
* OUTPUT:
*		status: pointer to MV_AUDIO_RECORD_STATUS structure
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvAudioRecordStatusGet(MV_AUDIO_RECORD_STATUS *status)
{
	status->I2SEnable =
			((MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG)&ARCR_RECORD_I2S_EN_MASK)?
			 MV_TRUE:MV_FALSE);

	status->mute =
			((MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG)&ARCR_RECORD_MUTE_MASK)?
			 MV_TRUE:MV_FALSE);

	status->pause =
			((MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG)&ARCR_RECORD_PAUSE_MASK)?
			 MV_TRUE:MV_FALSE);

	status->spdifEnable =
			((MV_REG_READ(MV_AUDIO_RECORD_CTRL_REG)&ARCR_RECORD_SPDIF_EN_MASK)?
			 MV_TRUE:MV_FALSE);

}
/*******************************************************************************
* mvSPDIFRecordTclockSet - Set T-clock for SPDIF
*
* DESCRIPTION:
*
* INPUT:
*       none
* OUTPUT:
*		none
* RETURN:
*       MV_OK on success , MV_NOT_SUPPORTED on non supported T-clock
*
*******************************************************************************/

/* SPDIF Recording Related*/
MV_STATUS	mvSPDIFRecordTclockSet()
{
	MV_U32 tclock = mvBoardTclkGet();
	MV_U32 reg = MV_REG_READ(MV_AUDIO_SPDIF_REC_GEN_REG);

	reg &= ~ASRGR_CORE_CLK_FREQ_MASK;

	switch (tclock)
	{
	case MV_BOARD_TCLK_133MHZ:
		reg |= ASRGR_CORE_CLK_FREQ_133MHZ;
		break;
	case MV_BOARD_TCLK_150MHZ:
		reg |= ASRGR_CORE_CLK_FREQ_150MHZ;
		break;
	case MV_BOARD_TCLK_166MHZ:
		reg |= ASRGR_CORE_CLK_FREQ_166MHZ;
		break;
	case MV_BOARD_TCLK_200MHZ:
		reg |= ASRGR_CORE_CLK_FREQ_200MHZ;
		break;
	default:
		mvOsPrintf("mvSPDIFRecordTclockSet: Not supported core clock %d\n",tclock);
		return MV_NOT_SUPPORTED;
	}

	MV_REG_WRITE(MV_AUDIO_SPDIF_REC_GEN_REG, reg);
	
	return MV_OK;

}
/*******************************************************************************
* mvSPDIFRecordTclockGet - Get T-clock for SPDIF
*
* DESCRIPTION:
*
* INPUT:
*       none
* OUTPUT:
*		none
* RETURN:
*       T-clock configured in the SPDIF.
*
*******************************************************************************/
MV_U32	mvSPDIFRecordTclockGet(MV_VOID)
{
	MV_U32 reg = (MV_REG_READ(MV_AUDIO_SPDIF_REC_GEN_REG) & ASRGR_CORE_CLK_FREQ_MASK);

	switch (reg)
	{
	case ASRGR_CORE_CLK_FREQ_133MHZ:
		return MV_BOARD_TCLK_133MHZ;
	case ASRGR_CORE_CLK_FREQ_150MHZ:
		return MV_BOARD_TCLK_150MHZ;
	case ASRGR_CORE_CLK_FREQ_166MHZ:
		return MV_BOARD_TCLK_166MHZ;
	case ASRGR_CORE_CLK_FREQ_200MHZ:
		return MV_BOARD_TCLK_200MHZ;
	}

	return 0;
}

/*******************************************************************************
* mvAudioRecordControlGet - Get SPDIF Recording status parameters
*
* DESCRIPTION:
*
* INPUT:
*       status: pointer to MV_SPDIF_RECORD_STATUS structure
* OUTPUT:
*		status: pointer to MV_SPDIF_RECORD_STATUS structure
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID	mvSPDIFRecordStatusGet(MV_SPDIF_RECORD_STATUS *status)
{
	status->freq =
		((MV_REG_READ(MV_AUDIO_SPDIF_REC_GEN_REG)&ASRGR_CORE_CLK_FREQ_MASK) >>
		 ASRGR_CORE_CLK_FREQ_OFFS);

	status->nonLinearPcm =
		((MV_REG_READ(MV_AUDIO_SPDIF_REC_GEN_REG)&ASRGR_NON_PCM_MASK)?
		 MV_TRUE:MV_FALSE);

	status->validPcm =
		((MV_REG_READ(MV_AUDIO_SPDIF_REC_GEN_REG)&ASRGR_VALID_PCM_INFO_MASK)?
		 MV_TRUE:MV_FALSE);

}

/* I2S Recording Related*/
/*******************************************************************************
* mvI2SRecordCntrlSet - Get I2S Recording status parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_I2S_RECORD_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_I2S_RECORD_CTRL structure
* RETURN:
*       MV_OK on success , MV_FAIL on fail.
*
*******************************************************************************/
MV_STATUS	mvI2SRecordCntrlSet(MV_I2S_RECORD_CTRL *ctrl)
{
	MV_U32 reg;

#if 0
	if (ctrl->sample > SAMPLE_16BIT)
	{
		mvOsPrintf("mvI2SRecordCntrlSet: Error , Illigal sample size %d\n",
				ctrl->sample);
		return MV_FAIL;
    }
#endif

	reg = MV_REG_READ(MV_AUDIO_I2S_REC_CTRL_REG);
	reg &= ~(AIRCR_I2S_RECORD_JUSTF_MASK|AIRCR_I2S_SAMPLE_SIZE_MASK);

	switch (ctrl->justf)
	{
	case I2S_JUSTIFIED:
	case LEFT_JUSTIFIED:
	case RIGHT_JUSTIFIED:
	case RISE_BIT_CLCK_JUSTIFIED:
		reg |= ctrl->justf << AIRCR_I2S_RECORD_JUSTF_OFFS;
		break;
	default:
		return MV_FAIL;
	}

	reg |= ctrl->sample << AIRCR_I2S_SAMPLE_SIZE_OFFS;

	MV_REG_WRITE(MV_AUDIO_I2S_REC_CTRL_REG, reg);
	return MV_OK;
}

/*******************************************************************************
* mvAudioRecordControlGet - Get I2S Recording status parameters
*
* DESCRIPTION:
*
* INPUT:
*       ctrl: pointer to MV_I2S_RECORD_CTRL structure
* OUTPUT:
*		ctrl: pointer to MV_I2S_RECORD_CTRL structure
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID	mvI2SRecordCntrlGet(MV_I2S_RECORD_CTRL *ctrl)
{
	ctrl->sample =
		((MV_REG_READ(MV_AUDIO_I2S_REC_CTRL_REG)&AIRCR_I2S_SAMPLE_SIZE_MASK) >>
		 AIRCR_I2S_SAMPLE_SIZE_OFFS);

	ctrl->justf =
		((MV_REG_READ(MV_AUDIO_I2S_REC_CTRL_REG)&AIRCR_I2S_RECORD_JUSTF_MASK) >>
		 AIRCR_I2S_RECORD_JUSTF_OFFS);


}


/*******************************************************************************
* audioBurstBytesNumGet - Convert Burst enum to bytes number
*
* DESCRIPTION:
*
* INPUT:
*       burst:  MV_AUDIO_BURST_SIZE  enum
* OUTPUT:
*		none
* RETURN:
*       number of burst bytes
*
*******************************************************************************/
static MV_U32 audioBurstBytesNumGet(MV_AUDIO_BURST_SIZE burst)
{
	switch (burst)
	{
	case AUDIO_32BYTE_BURST:
		return 32;
	case AUDIO_128BYTE_BURST:
		return 128;
	default:
		return 0xffffffff;
	}
}

