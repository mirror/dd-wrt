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
#ifndef __INCMVAudioH
#define __INCMVAudioH

#include "mvCommon.h"
#include "audio/mvAudioRegs.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"

/*********************************/
/* General enums and structures */
/*********************************/
/* Type of Audio operations*/
typedef enum _mvAudioOperation
{
		AUDIO_PLAYBACK = 0,
		AUDIO_RECORD = 1

}MV_AUDIO_OP;

typedef struct _mvAudioFreqData
{
	MV_AUDIO_FREQ	baseFreq; 	/* Control FS, selects the base frequency of the DCO */
	MV_U32			offset; 	/* Offset control in which each step equals to 0.9536 ppm */

} MV_AUDIO_FREQ_DATA;


/*********************************/
/* Play Back related structures */
/*********************************/


typedef struct _mvAudioPlaybackCtrl
{
	MV_AUDIO_BURST_SIZE 	burst;			/* Specifies the Burst Size of the DMA */
	MV_BOOL 				loopBack;		/* When Loopback is enabled, playback
											data is looped back to be recorded */
	MV_AUDIO_PLAYBACK_MONO	monoMode;		/* Mono Mode is used */
	MV_U32					bufferPhyBase;	/* Physical Address of DMA buffer */
	MV_U32					bufferSize;		/* Size of DMA buffer */
    MV_U32                  intByteCount;   /* Number of bytes after which an
                                            interrupt will be issued.*/
    MV_AUDIO_SAMPLE_SIZE	sampleSize;	    /* Playback Sample Size*/
}MV_AUDIO_PLAYBACK_CTRL;

typedef struct _mvAudioPlaypackStatus
{
	MV_BOOL	muteI2S;
	MV_BOOL	enableI2S;
	MV_BOOL	muteSPDIF;
	MV_BOOL	enableSPDIF;
	MV_BOOL pause;

}MV_AUDIO_PLAYBACK_STATUS;


typedef struct _mvSpdifPlaybackCtrl
{
	MV_BOOL 	nonPcm;				 	/* PCM or non-PCM mode*/
	MV_BOOL 	validity;			 	/* Validity bit value when using
										registers (userBitsFromMemory=0)*/
	MV_BOOL		underrunData; 			/* If true send last frame on mute/pause/underrun
										otherwise send 24 binary*/
	MV_BOOL 	userBitsFromMemory;  	/* otherwise from intenal registers*/
	MV_BOOL		validityFromMemory; 	/* otherwise from internal registers*/
	MV_BOOL		blockStartInternally;	/* When user and valid bits are form registers
										then this bit should be zero */
}MV_SPDIF_PLAYBACK_CTRL;

typedef struct _mvI2SPlaybackCtrl
{
	MV_AUDIO_SAMPLE_SIZE		sampleSize;
	MV_AUDIO_I2S_JUSTIFICATION	justification;
	MV_BOOL 					sendLastFrame;	/* If true send last frame on mute/pause/underrun
												otherwise send 64 binary*/
}MV_I2S_PLAYBACK_CTRL;


/*********************************/
/* Recording  related structures */
/*********************************/

typedef struct _mvAudioRecordCtrl
{
	MV_AUDIO_BURST_SIZE 	burst;			/* Recording DMA Burst Size */
	MV_AUDIO_SAMPLE_SIZE	sampleSize;		/*Recording Sample Size */
    MV_BOOL					mono;			/* If true then recording mono else recording stereo*/
	MV_AUDIO_RECORD_MONO	monoChannel;	/* Left or right moono*/
	MV_U32					bufferPhyBase;	/* Physical Address of DMA buffer */
	MV_U32					bufferSize;		/* Size of DMA buffer */

    MV_U32                  intByteCount;   /* Number of bytes after which an
                                            interrupt will be issued.*/


}MV_AUDIO_RECORD_CTRL;

typedef struct _mvAudioRecordStatus
{
	MV_BOOL	mute;
	MV_BOOL pause;
	MV_BOOL	spdifEnable;
	MV_BOOL	I2SEnable;

}MV_AUDIO_RECORD_STATUS;


typedef struct _mvSPDIFRecordStatus
{
	MV_BOOL 				nonLinearPcm;		/* pcm non-pcm*/
	MV_BOOL					validPcm;	/* valid non-valid pcm*/
	MV_AUDIO_SAMPLE_FREQ	freq;		/* sampled frequency*/

}MV_SPDIF_RECORD_STATUS;

typedef struct _mvI2SRecordCntrl
{
	MV_AUDIO_SAMPLE_SIZE		sample;		/* I2S Recording Sample Size*/
	MV_AUDIO_I2S_JUSTIFICATION	justf;
}MV_I2S_RECORD_CTRL;



/*********************************/
/* Usefull Macros 				*/
/*
 -- Clocks Control and Status related --
mvAudioIsDcoLocked()
mvAudioIsSpcrLocked()
mvAudioIsPllLocked()
mvAudioAllCountersClear()
mvAudioPlayCounterClear()
mvAudioRecCounterClear()
mvAudioAllCountersStart()
mvAudioPlayCounterStart()
mvAudioRecCounterStart()
mvAudioAllCountersStop()
mvAudioPlayCounterStop()
mvAudioRecCounterStop()

 -- PlayBack related --
mvAudioIsPlaybackBusy()
mvAudioI2SPlaybackMute(mute)
mvAudioI2SPlaybackEnable(enable)
mvAudioSPDIFPlaybackMute(mute)
mvAudioSPDIFPlaybackEnable(enable)
mvAudioPlaybackPause(pause)

---- Recording ---
mvAudioSPDIFRecordingEnable(enable)
mvAudioI2SRecordingEnable(enable)
mvAudioRecordMute(mute)
mvAudioRecordPause(pause)

********************************/




/* Clocks Control and Status related*/
#define mvAudioIsDcoLocked()	\
	(ASDSR_DCO_LOCK_MASK & MV_REG_READ(MV_AUDIO_SPCR_DCO_STATUS_REG))
#define	mvAudioIsSpcrLocked()	\
	(ASDSR_SPCR_LOCK_MASK & MV_REG_READ(AUDIO_SPCR_DCO_STATUS_REG))
#define	mvAudioIsPllLocked()	\
	(ASDSR_PLL_LOCK_MASK & MV_REG_READ(AUDIO_SPCR_DCO_STATUS_REG))

#define mvAudioAllCountersClear()	\
	(MV_REG_BIT_SET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_CLR_REC_CNTR_MASK|ASCCR_CLR_PLAY_CNTR_MASK)))
#define mvAudioPlayCounterClear()	\
	(MV_REG_BIT_SET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_CLR_PLAY_CNTR_MASK)))
#define mvAudioRecCounterClear()	\
	(MV_REG_BIT_SET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_CLR_REC_CNTR_MASK)))

#define	mvAudioAllCountersStart()	\
	(MV_REG_BIT_SET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_ACTIVE_PLAY_CNTR_MASK|ASCCR_ACTIVE_REC_CNTR_MASK)))
#define mvAudioPlayCounterStart()	\
	(MV_REG_BIT_SET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_ACTIVE_PLAY_CNTR_MASK)))
#define mvAudioRecCounterStart()	\
	(MV_REG_BIT_SET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_ACTIVE_REC_CNTR_MASK)))

#define	mvAudioAllCountersStop()	\
	(MV_REG_BIT_RESET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_ACTIVE_PLAY_CNTR_MASK|ASCCR_ACTIVE_REC_CNTR_MASK)))
#define mvAudioPlayCounterStop()	\
	(MV_REG_BIT_RESET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_ACTIVE_PLAY_CNTR_MASK)))
#define mvAudioRecCounterStop()	\
	(MV_REG_BIT_RESET(MV_AUDIO_SAMPLE_CNTR_CTRL_REG,(ASCCR_ACTIVE_REC_CNTR_MASK)))

/* Audio PlayBack related*/
#define	mvAudioIsPlaybackBusy()	\
	(APCR_PLAY_BUSY_MASK & MV_REG_READ(MV_AUDIO_PLAYBACK_CTRL_REG))

#define	mvAudioI2SPlaybackMute(mute)	\
	(void)((mute)?(MV_REG_BIT_SET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_I2S_MUTE_MASK)):	\
			 MV_REG_BIT_RESET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_I2S_MUTE_MASK))
#define	mvAudioI2SPlaybackEnable(enable)	\
	 (void)((enable)?(MV_REG_BIT_SET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_I2S_ENABLE_MASK)):	\
			  MV_REG_BIT_RESET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_I2S_ENABLE_MASK))

#define	mvAudioSPDIFPlaybackMute(mute)	\
	(void)((mute)?(MV_REG_BIT_SET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_SPDIF_MUTE_MASK)):	\
			 MV_REG_BIT_RESET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_SPDIF_MUTE_MASK))
#define	mvAudioSPDIFPlaybackEnable(enable)	\
	 (void)((enable)?(MV_REG_BIT_SET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_SPDIF_ENABLE_MASK)):	\
			  MV_REG_BIT_RESET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_SPDIF_ENABLE_MASK))

#define	mvAudioAllIfPlaybackEnable(enable)	\
	 (void)((enable)?    \
            (MV_REG_BIT_SET(MV_AUDIO_PLAYBACK_CTRL_REG,     \
                            (APCR_PLAY_I2S_ENABLE_MASK | APCR_PLAY_SPDIF_ENABLE_MASK))):	\
			  MV_REG_BIT_RESET(MV_AUDIO_PLAYBACK_CTRL_REG,  \
                            (APCR_PLAY_I2S_ENABLE_MASK | APCR_PLAY_SPDIF_ENABLE_MASK)))

#define	mvAudioPlaybackPause(pause)	\
	 (void)((pause)?(MV_REG_BIT_SET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_PAUSE_MASK)):	\
			  MV_REG_BIT_RESET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_PLAY_PAUSE_MASK))

#define	mvAudioPlaybackLoopbackEnable(enable)	\
	 (void)((enable)?(MV_REG_BIT_SET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_LOOPBACK_MASK)):	\
			  MV_REG_BIT_RESET(MV_AUDIO_PLAYBACK_CTRL_REG,APCR_LOOPBACK_MASK))


/* Audio Recording*/
#define	mvAudioSPDIFRecordingEnable(enable)	\
	 (void)((enable)?(MV_REG_BIT_SET(MV_AUDIO_RECORD_CTRL_REG,ARCR_RECORD_SPDIF_EN_MASK)):	\
			  MV_REG_BIT_RESET(MV_AUDIO_RECORD_CTRL_REG,ARCR_RECORD_SPDIF_EN_MASK))

#define	mvAudioI2SRecordingEnable(enable)	\
	 (void)((enable)?(MV_REG_BIT_SET(MV_AUDIO_RECORD_CTRL_REG,ARCR_RECORD_I2S_EN_MASK)):	\
			  MV_REG_BIT_RESET(MV_AUDIO_RECORD_CTRL_REG,ARCR_RECORD_I2S_EN_MASK))

#define mvAudioRecordMute(mute)	\
	 (void)((mute)?(MV_REG_BIT_SET(MV_AUDIO_RECORD_CTRL_REG,ARCR_RECORD_MUTE_MASK)):	\
			  MV_REG_BIT_RESET(MV_AUDIO_RECORD_CTRL_REG,ARCR_RECORD_MUTE_MASK))

#define mvAudioRecordPause(pause)	\
	 (void)((pause)?(MV_REG_BIT_SET(MV_AUDIO_RECORD_CTRL_REG,ARCR_RECORD_PAUSE_MASK)):	\
			  MV_REG_BIT_RESET(MV_AUDIO_RECORD_CTRL_REG,ARCR_RECORD_PAUSE_MASK))



/*********************************/
/* Functions API 				*/
/*********************************/

MV_VOID mvAudioHalInit(void);

/* Clocks Control and Status related*/
MV_STATUS mvAudioDCOCtrlSet(MV_AUDIO_FREQ_DATA *dcoCtrl);
MV_VOID mvAudioDCOCtrlGet(MV_AUDIO_FREQ_DATA *dcoCtrl);
MV_VOID mvAudioSpcrCtrlGet(MV_AUDIO_FREQ_DATA *spcrCtrl);

/* Audio PlayBack related*/
MV_STATUS mvAudioPlaybackControlSet(MV_AUDIO_PLAYBACK_CTRL *ctrl);
MV_VOID mvAudioPlaybackControlGet(MV_AUDIO_PLAYBACK_CTRL *ctrl);
MV_VOID mvAudioPlaybackStatusGet(MV_AUDIO_PLAYBACK_STATUS *status);

/* Audio SPDIF PlayBack related*/
MV_VOID mvSPDIFPlaybackCtrlSet(MV_SPDIF_PLAYBACK_CTRL *ctrl);
MV_VOID mvSPDIFPlaybackCtrlGet(MV_SPDIF_PLAYBACK_CTRL *ctrl);

/* Audio I2S PlayBack related*/
MV_STATUS mvI2SPlaybackCtrlSet(MV_I2S_PLAYBACK_CTRL *ctrl);
MV_VOID mvI2SPlaybackCtrlGet(MV_I2S_PLAYBACK_CTRL *ctrl);

/* Audio Recording*/
MV_STATUS mvAudioRecordControlSet(MV_AUDIO_RECORD_CTRL *ctrl);
MV_VOID mvAudioRecordControlGet(MV_AUDIO_RECORD_CTRL *ctrl);
MV_VOID mvAudioRecordStatusGet(MV_AUDIO_RECORD_STATUS *status);

/* SPDIF Recording Related*/
MV_STATUS	mvSPDIFRecordTclockSet(MV_VOID);
MV_U32	mvSPDIFRecordTclockGet(MV_VOID);
MV_VOID	mvSPDIFRecordStatusGet(MV_SPDIF_RECORD_STATUS *status);

/* I2S Recording Related*/
MV_STATUS	mvI2SRecordCntrlSet(MV_I2S_RECORD_CTRL *ctrl);
MV_VOID	mvI2SRecordCntrlGet(MV_I2S_RECORD_CTRL *ctrl);

#endif
