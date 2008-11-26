/*
 *
 *	Marvell Orion Alsa Sound driver
 *
 *	Author: Maen Suleiman
 *	Copyright (C) 2008 Marvell Ltd.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <sound/driver.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/asoundef.h>
#include <sound/asound.h>

#include "twsi/mvTwsi.h"
#include "audio/dac/mvCLAudioCodec.h"
#include "boardEnv/mvBoardEnvLib.h"



/*
 * Initialize the audio decoder.
 */
int
cs42l51_init(void)
{
    MV_AUDIO_CODEC_DEV codec_params;
    unsigned char reg_data;
    

    codec_params.ADCMode = MV_I2S_MODE;
    codec_params.DACDigitalIFFormat = MV_I2S_UP_TO_24_BIT;
    codec_params.twsiSlave.moreThen256 = MV_FALSE;
    codec_params.twsiSlave.validOffset = MV_TRUE;
    codec_params.twsiSlave.slaveAddr.address = mvBoardA2DTwsiAddrGet();
    codec_params.twsiSlave.slaveAddr.type = mvBoardA2DTwsiAddrTypeGet();
    if(mvCLAudioCodecInit(&codec_params) == MV_FALSE)
    {
        printk("Error - Cannot initialize audio decoder.at address =0x%x",
				codec_params.twsiSlave.slaveAddr.address);
        return -1;
    }

    /* Use the signal processor.               */
    mvCLAudioCodecRegSet(&codec_params,0x9,0x40);
    
    /* Unmute PCM-A & PCM-B and set default      */
    mvCLAudioCodecRegSet(&codec_params,0x10,0x60);
    mvCLAudioCodecRegSet(&codec_params,0x11,0x60);

    /* default for AOUTx*/
    mvCLAudioCodecRegSet(&codec_params,0x16,0x05);
    mvCLAudioCodecRegSet(&codec_params,0x17,0x05);

    /* swap channels */
    mvCLAudioCodecRegSet(&codec_params,0x18,0xff);
    if (0) {
	    int i;
	    for (i=1; i<= 0x21 ; i++) {
		    reg_data = mvCLAudioCodecRegGet(&codec_params,i);
		    printk("CLS reg=0x%02x val=0x%02x\n",i,reg_data);
	    }
	    
    }
    
    return 0;
}

#define MVAUD_NUM_VOLUME_STEPS  (40)
static MV_U8 auddec_volume_mapping[MVAUD_NUM_VOLUME_STEPS] =
{
	0x19,	0xB2,	0xB7,	0xBD,	0xC3,	0xC9,	0xCF,	0xD5,
	0xD8,	0xE1,	0xE7,	0xED,	0xF3,	0xF9,	0xFF,	0x00,	
	0x01,	0x02,	0x03,	0x04,	0x05,	0x06,	0x07,	0x08,	
	0x09,	0x0A,	0x0B,	0x0C,	0x0D,	0x0E,	0x0F,	0x10,	
	0x11,	0x12,	0x13,	0x14,	0x15,	0x16,	0x17,	0x18
};


/*
 * Get the audio decoder volume for both channels.
 * 0 is lowest volume, MVAUD_NUM_VOLUME_STEPS-1 is the highest volume.
 */

void
cs42l51_vol_get(MV_U8 *vol_list)
{
    MV_AUDIO_CODEC_DEV  codec_params;
    MV_U8   reg_data;
    MV_U32  i;
    MV_U32  vol_idx;

    codec_params.ADCMode = MV_I2S_MODE;
    codec_params.DACDigitalIFFormat = MV_I2S_UP_TO_24_BIT;
    codec_params.twsiSlave.moreThen256 = MV_FALSE;
    codec_params.twsiSlave.validOffset = MV_TRUE;
	codec_params.twsiSlave.slaveAddr.address = mvBoardA2DTwsiAddrGet();
	codec_params.twsiSlave.slaveAddr.type = mvBoardA2DTwsiAddrTypeGet();


    for(vol_idx = 0; vol_idx < 2; vol_idx++)
    {
        reg_data = mvCLAudioCodecRegGet(&codec_params,0x16 + vol_idx);

       /*printk("\tVolume was get: 0x%x.\n",
                                reg_data);*/

        /* Look for the index that mapps to this dB value.  */
        for(i = 0; i < MVAUD_NUM_VOLUME_STEPS; i++)
        {
		if (reg_data == auddec_volume_mapping[i])
			break;
		if (( auddec_volume_mapping[i] > auddec_volume_mapping[MVAUD_NUM_VOLUME_STEPS-1]) 
					&& (reg_data > auddec_volume_mapping[i]) 
					&& (reg_data < auddec_volume_mapping[i+1]))
			break;
		 
        }

        vol_list[vol_idx] = i;
       /*printk("\tvol_list[%d] = %d.\n",vol_idx,
                                vol_list[vol_idx]);*/

    }

    return;
}


/*
 * Set the audio decoder volume for both channels.
 * 0 is lowest volume, MVAUD_NUM_VOLUME_STEPS-1 is the highest volume.
 */
void
cs42l51_vol_set(MV_U8 *vol_list)
{
    MV_AUDIO_CODEC_DEV  codec_params;
    MV_U32  vol_idx;

    codec_params.ADCMode = MV_I2S_MODE;
    codec_params.DACDigitalIFFormat = MV_I2S_UP_TO_24_BIT;
    codec_params.twsiSlave.moreThen256 = MV_FALSE;
    codec_params.twsiSlave.validOffset = MV_TRUE;
    codec_params.twsiSlave.slaveAddr.address = mvBoardA2DTwsiAddrGet();
    codec_params.twsiSlave.slaveAddr.type = mvBoardA2DTwsiAddrTypeGet();

   
    for(vol_idx = 0; vol_idx < 2; vol_idx++)
    {
        /*printk("\tvol_list[%d] = %d.\n",vol_idx,
                                vol_list[vol_idx]);*/

        if(vol_list[vol_idx] >= MVAUD_NUM_VOLUME_STEPS)
		vol_list[vol_idx] = MVAUD_NUM_VOLUME_STEPS -1;

        mvCLAudioCodecRegSet(&codec_params,0x16 + vol_idx,
                             auddec_volume_mapping[vol_list[vol_idx]]);

        /*printk("\tVolume was set to 0x%x.\n",
                                auddec_volume_mapping[vol_list[vol_idx]]);*/
    }

    return;
}

