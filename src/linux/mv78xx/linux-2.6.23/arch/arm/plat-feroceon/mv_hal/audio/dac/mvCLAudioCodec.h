
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

#ifndef __INCmvCLAudioCodech
#define __INCmvCLAudioCodech

#include "mvTypes.h"
#include "twsi/mvTwsi.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"
#include "boardEnv/mvBoardEnvSpec.h"

#define  MV_CL_AUDIO_CODEC_CHIP_ID   0x1B
#define  MV_CL_AUDIO_CODEC_REV_ID    0x1

/* Selects the digital interface format used for the data in on SDIN. */
typedef enum _mvDACDigitalIFFormat
{
    MV_L_JUSTIFIED_UP_TO_24_BIT,
    MV_I2S_UP_TO_24_BIT,
    MV_R_JUSTIFIED_UP_TO_24_BIT,
    MV_R_JUSTIFIED_20_BIT,
    MV_R_JUSTIFIED_18_BIT,
    MV_R_JUSTIFIED_16_BIT

} MV_DAC_DIGITAL_IF_FORMAT;

/* Selects either the I2S or Left-Justified digital interface format for the
   data on SDOUT. */
typedef enum _mvADCMode
{
    MV_LEFT_JUSTIFIED_MODE,
    MV_I2S_MODE
} MV_ADC_MODE;


/* Cirrus Logic device structure */
typedef struct _mvAudioCodecDev
{
    /* MUST be set by user!!!   */
	MV_TWSI_SLAVE             twsiSlave;
    MV_DAC_DIGITAL_IF_FORMAT  DACDigitalIFFormat;
    MV_ADC_MODE               ADCMode;

    /* Set by driver */
	MV_U8  nChipID;
	MV_U8  nRevID;

} MV_AUDIO_CODEC_DEV;

/* Initialize the Cirrus Logic device */
MV_BOOL mvCLAudioCodecInit(MV_AUDIO_CODEC_DEV *pCodecDev);

/* Function to control output volume (playback) */
MV_VOID mvCLAudioCodecOutputVolumeSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_8  nVolume);
MV_U8   mvCLAudioCodecOutputVolumeGet(MV_AUDIO_CODEC_DEV *pCodecDev);
MV_VOID mvCLAudioCodecOutputVolumeMute(MV_AUDIO_CODEC_DEV *pCodecDev, MV_BOOL bMute);

/* Function to control input volume (record) */
MV_VOID mvCLAudioCodecInputVolumeSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_8  nVolume);
MV_U8   mvCLAudioCodecInputVolumeGet(MV_AUDIO_CODEC_DEV *pCodecDev);
MV_VOID mvCLAudioCodecInputVolumeMute(MV_AUDIO_CODEC_DEV *pCodecDev, MV_BOOL bMute);

/* Function to control output tone */
MV_VOID mvCLAudioCodecTrebleSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_8  nTreble);
MV_U8   mvCLAudioCodecTrebleGet(MV_AUDIO_CODEC_DEV *pCodecDev);
MV_VOID mvCLAudioCodecBassSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_8  nBass);
MV_U8   mvCLAudioCodecBassGet(MV_AUDIO_CODEC_DEV *pCodecDev);

/* Function to access the Cirrus Logic CODEC registers */
MV_VOID mvCLAudioCodecRegSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_U8  nOffset, MV_U8 nData);
MV_U8   mvCLAudioCodecRegGet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_U8  nOffset);
MV_VOID mvCLAudioCodecRegBitsSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_U8  nOffset, MV_U8 nBits);
MV_VOID mvCLAudioCodecRegBitsReset(MV_AUDIO_CODEC_DEV *pCodecDev, MV_U8  nOffset, MV_U8 nBits);


#endif  /* __INCmvCLAudioCodech */

