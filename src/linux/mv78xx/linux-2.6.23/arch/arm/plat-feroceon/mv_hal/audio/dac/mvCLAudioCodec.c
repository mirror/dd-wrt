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

#include "mvCLAudioCodec.h"
#include "mvCLAudioCodecRegs.h"

/*******************************************************************************
* mvCLAudioCodecInit - Initizlize the Cirrus Logic device
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
* OUTPUT:
*		None
* RETURN:
*       MV_TRUE or MV_FALSE.
*
*******************************************************************************/
MV_BOOL mvCLAudioCodecInit(MV_AUDIO_CODEC_DEV *pCodecDev)
{
    MV_U8    nData;

    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return MV_FALSE;
    }

    /* Verify chip ID and revision */
    nData = mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_ID_REG);
    if((MV_CL_AUDIO_CODEC_CHIP_ID != (nData >> 3)) ||
       (MV_CL_AUDIO_CODEC_REV_ID  != (nData & 0x7)))
    {
        mvOsPrintf("%s: Error - Invalid Cirrus Logic chip/rev ID!\n",__FUNCTION__);
        return MV_FALSE;
    }

    /* Set the digital Interface format */
    nData = mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_IF_CTRL_REG);
    nData &= ~(0x7<<3);
    nData |= (pCodecDev->DACDigitalIFFormat << 3);
    mvCLAudioCodecRegSet(pCodecDev,CL_AUDIO_CODEC_IF_CTRL_REG,nData);

    /* Set the ADC Mode */
    if(MV_LEFT_JUSTIFIED_MODE == pCodecDev->ADCMode)
        mvCLAudioCodecRegBitsReset(pCodecDev,CL_AUDIO_CODEC_IF_CTRL_REG,BIT2);
    else
        mvCLAudioCodecRegBitsSet(pCodecDev,CL_AUDIO_CODEC_IF_CTRL_REG,BIT2);


    return MV_TRUE;
}

/*******************************************************************************
* mvCLAudioCodecOutputVolumeSet - Set the Cirrus Logic output volume of OUTA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       nVolume  : Volume level.
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvCLAudioCodecOutputVolumeSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_8  nVolume)
{
    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }
    mvCLAudioCodecRegSet(pCodecDev,CL_AUDIO_CODEC_VOL_OUTA_CTRL_REG,nVolume);
}

/*******************************************************************************
* mvCLAudioCodecOutputVolumeGet - Get the Cirrus Logic output volume of OUTA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
* OUTPUT:
*       None
* RETURN:
*		Volume level.
*
*******************************************************************************/
MV_U8    mvCLAudioCodecOutputVolumeGet(MV_AUDIO_CODEC_DEV *pCodecDev)
{
    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return 0;
    }
    return mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_VOL_OUTA_CTRL_REG);
}

/*******************************************************************************
* mvCLAudioCodecOutputVolumeMute - Mute the Cirrus Logic output volume of OUTA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       bMute    : MV_TRUE for mute, MV_FALSE to un-mute
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvCLAudioCodecOutputVolumeMute(MV_AUDIO_CODEC_DEV *pCodecDev, MV_BOOL bMute)
{
    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }
    if(MV_TRUE == bMute)
        mvCLAudioCodecRegBitsSet(pCodecDev,CL_AUDIO_CODEC_DAC_OUTPUT_CTRL_REG,BIT0);
    else
        mvCLAudioCodecRegBitsReset(pCodecDev,CL_AUDIO_CODEC_DAC_OUTPUT_CTRL_REG,BIT0);
}

/*******************************************************************************
* mvCLAudioCodecInputVolumeSet - Set the Cirrus Logic input volume of INA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       nVolume  : Volume level.
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvCLAudioCodecInputVolumeSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_8  nVolume)
{
    MV_U8 nData;

    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }
    nData = mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_PGAA_VOL_CTRL_REG);
    nData &= ~0x1f;
    nData |= (0x1f & nVolume);
    mvCLAudioCodecRegSet(pCodecDev,CL_AUDIO_CODEC_PGAA_VOL_CTRL_REG,nData);
}

/*******************************************************************************
* mvCLAudioCodecInputVolumeGet - Get the Cirrus Logic input volume of INA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
* OUTPUT:
*		None
* RETURN:
*       Volume level.
*
*******************************************************************************/
MV_U8    mvCLAudioCodecInputVolumeGet(MV_AUDIO_CODEC_DEV *pCodecDev)
{
    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return 0;
    }
    return mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_PGAA_VOL_CTRL_REG) & 0x1f;
}

/*******************************************************************************
* mvCLAudioCodecInputVolumeMute - Mute the Cirrus Logic input volume of INA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       bMute    : MV_TRUE for mute, MV_FALSE to un-mute
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvCLAudioCodecInputVolumeMute(MV_AUDIO_CODEC_DEV *pCodecDev,
                                      MV_BOOL bMute)
{
    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }
    if(MV_TRUE == bMute)
        mvCLAudioCodecRegBitsSet(pCodecDev,CL_AUDIO_CODEC_ADC_INPUT_INV_MUTE_REG,
                                 BIT0);
    else
        mvCLAudioCodecRegBitsReset(pCodecDev,CL_AUDIO_CODEC_ADC_INPUT_INV_MUTE_REG,
                                   BIT0);
}

/*******************************************************************************
* mvCLAudioCodecTrebleSet - Set the Cirrus Logic output treble of INA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       nTreble  : treble value.
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvCLAudioCodecTrebleSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_8  nTreble)
{
    MV_U8 nData;

    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }
    nData = mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_TONE_CTRL_REG);
    nData &= 0xF;
    nData |= (nTreble<<4);
    mvCLAudioCodecRegSet(pCodecDev,CL_AUDIO_CODEC_TONE_CTRL_REG,nData);
}

/*******************************************************************************
* mvCLAudioCodecTrebleGet - Get the Cirrus Logic output treble of INA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
* OUTPUT:
*		None
* RETURN:
*       treble value
*
*******************************************************************************/
MV_U8    mvCLAudioCodecTrebleGet(MV_AUDIO_CODEC_DEV *pCodecDev)
{
    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return 0;
    }
    return mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_TONE_CTRL_REG)>>4;
}
/*******************************************************************************
* mvCLAudioCodecBassSet - Set the Cirrus Logic output bass of INA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       nBass    : Bass level.
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvCLAudioCodecBassSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_8  nBass)
{
    MV_U8 nData;

    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }
    nData = mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_TONE_CTRL_REG);
    nData &= 0xf0;
    nData |= nBass;
    mvCLAudioCodecRegSet(pCodecDev,CL_AUDIO_CODEC_TONE_CTRL_REG,nData);
}

/*******************************************************************************
* mvCLAudioCodecBassGet - Get the Cirrus Logic output bass of INA
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
* OUTPUT:
*		None
* RETURN:
*       Bass level
*
*******************************************************************************/
MV_U8    mvCLAudioCodecBassGet(MV_AUDIO_CODEC_DEV *pCodecDev)
{
    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return 0;
    }
    return mvCLAudioCodecRegGet(pCodecDev,CL_AUDIO_CODEC_TONE_CTRL_REG)& 0xF;
}

/*******************************************************************************
* mvCLAudioCodecRegSet - Set Cirrus Logic register value
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       nOffset  : register offset
*       nData    : register data
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvCLAudioCodecRegSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_U8  nOffset,
                             MV_U8 nData)
{
    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }
    pCodecDev->twsiSlave.offset = nOffset;
    if(mvTwsiWrite(&pCodecDev->twsiSlave,&nData,1) != MV_OK)
    {
        mvOsPrintf("%s: Error while writing register!\n",__FUNCTION__);
        return;
    }
}

/*******************************************************************************
* mvCLAudioCodecRegGet - Set Cirrus Logic register value
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       nOffset  : register offset
* OUTPUT:
*		None
* RETURN:
*       register data
*
*******************************************************************************/
MV_U8   mvCLAudioCodecRegGet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_U8  nOffset)
{
    MV_U8   nData;

    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return 0;
    }
    pCodecDev->twsiSlave.offset = nOffset;
    if(mvTwsiRead(&pCodecDev->twsiSlave,&nData,1) != MV_OK)
    {
        mvOsPrintf("%s: Error while reading register!\n",__FUNCTION__);
        return 0;
    }
    return nData;
}

/*******************************************************************************
* mvCLAudioCodecRegBitsSet - Set Cirrus Logic register bits value
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       nOffset  : register offset
*       nBits    : register bits
* OUTPUT:
*		None
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvCLAudioCodecRegBitsSet(MV_AUDIO_CODEC_DEV *pCodecDev, MV_U8  nOffset,
                                 MV_U8 nBits)
{
    MV_U8  nData;

    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }

    pCodecDev->twsiSlave.offset = nOffset;
    nData = mvCLAudioCodecRegGet(pCodecDev,nOffset);
    nData |= nBits;
    mvCLAudioCodecRegSet(pCodecDev,nOffset,nData);

}

/*******************************************************************************
* mvCLAudioCodecRegGet - Reset Cirrus Logic register bits value
*
* DESCRIPTION:
*
* INPUT:
*       pCodecDev: pointer to MV_AUDIO_CODEC_DEV structure.
*       nOffset  : register offset
*       nBits    : register bits
* OUTPUT:
*		None
* RETURN:
*       register data
*
*******************************************************************************/
MV_VOID   mvCLAudioCodecRegBitsReset(MV_AUDIO_CODEC_DEV *pCodecDev, MV_U8  nOffset, MV_U8 nBits)
{
    MV_U8  nData;

    if(NULL == pCodecDev)
    {
        mvOsPrintf("%s: Error - pCodecDev = NULL!\n",__FUNCTION__);
        return;
    }

    pCodecDev->twsiSlave.offset = nOffset;
    nData = mvCLAudioCodecRegGet(pCodecDev,nOffset);
    nData &= ~nBits;
    mvCLAudioCodecRegSet(pCodecDev,nOffset,nData);
}
