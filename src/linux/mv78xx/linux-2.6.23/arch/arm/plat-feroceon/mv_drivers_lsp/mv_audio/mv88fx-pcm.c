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
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <asm/dma.h>
#include <asm/io.h>
#include <asm/scatterlist.h>
#include <asm/sizes.h>
#include "mv88fx-pcm.h"
/*#define AUDIO_REG_BASE	0x0*/
#include "audio/mvAudioRegs.h"


int mv88fx_snd_hw_init(struct mv88fx_snd_chip	*chip);
int mv88fx_snd_hw_playback_set(struct mv88fx_snd_chip	*chip);
int mv88fx_snd_hw_capture_set(struct mv88fx_snd_chip	*chip);

struct mv88fx_snd_chip	*chip = NULL;
/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
int test_memory(struct mv88fx_snd_chip *chip,
		unsigned int base,
		unsigned int size)
{
	unsigned int i;

	for (i=0; i<6; i++) {

		/* check if we get to end */
		if ((chip->mem_array[i].base == 0) && 
		   (chip->mem_array[i].size == 0)) 
			break;

		/* check if we fit into one memory window only */
		if ((base >= chip->mem_array[i].base) &&
		   ((base + size) <=  chip->mem_array[i].base + chip->mem_array[i].size))
			return 1;

	}

	return 0;
}

/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/



void devdma_hw_free(struct device *dev, struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_dma_buffer *buf = runtime->dma_buffer_p;

	if (runtime->dma_area == NULL)
		return;

	if (buf != &substream->dma_buffer) {
		kfree(runtime->dma_buffer_p);
	}

	snd_pcm_set_runtime_buffer(substream, NULL);
}

int devdma_hw_alloc(struct device *dev, struct snd_pcm_substream *substream, size_t size)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_dma_buffer *buf = runtime->dma_buffer_p;
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); 

	int ret = 0;

	if (buf) {
		if (buf->bytes >= size) {
			mv88fx_snd_debug(KERN_ERR "devdma_hw_alloc:buf->bytes >= size\n");
			goto out;
		}
		devdma_hw_free(dev, substream);
	}

	if (substream->dma_buffer.area != NULL && substream->dma_buffer.bytes >= size) {
		buf = &substream->dma_buffer;
	} else {
		buf = kmalloc(sizeof(struct snd_dma_buffer), GFP_KERNEL);
		if (!buf) {
			mv88fx_snd_debug(KERN_ERR "devdma_hw_alloc:buf == NULL\n");
			goto nomem;
		}

		buf->dev.type = SNDRV_DMA_TYPE_DEV;
		buf->dev.dev = dev;
		buf->area = audio_stream->area;
		buf->addr = audio_stream->addr;
		buf->bytes = size;
		buf->private_data = NULL;

		if (!buf->area) {
			mv88fx_snd_debug(KERN_ERR "devdma_hw_alloc:buf->area == NULL\n");
			goto free;
		}
	}
	snd_pcm_set_runtime_buffer(substream, buf);
	ret = 1;
 out:
	runtime->dma_bytes = size;
	return ret;

 free:
	kfree(buf);
 nomem:
	return -ENOMEM;
}

int devdma_mmap(struct device *dev, struct snd_pcm_substream *substream, struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	return dma_mmap_coherent(dev, vma, runtime->dma_area, runtime->dma_addr, runtime->dma_bytes);
}


/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/

/*
 * hw preparation for spdif
 */


static int mv88fx_snd_spdif_mask_info(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_IEC958;
	uinfo->count = 1;
	return 0;
}

static int mv88fx_snd_spdif_mask_get(struct snd_kcontrol *kcontrol,
				     struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.iec958.status[0] = 0xff;
	ucontrol->value.iec958.status[1] = 0xff;
	ucontrol->value.iec958.status[2] = 0xff;
	ucontrol->value.iec958.status[3] = 0xff;
	return 0;
}

static struct snd_kcontrol_new mv88fx_snd_spdif_mask  =
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,CON_MASK),
	.info =		mv88fx_snd_spdif_mask_info,
	.get =		mv88fx_snd_spdif_mask_get,
};

static int mv88fx_snd_spdif_stream_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_IEC958;
	uinfo->count = 1;
	return 0;
}

static int mv88fx_snd_spdif_stream_get(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct mv88fx_snd_chip *chip = snd_kcontrol_chip(kcontrol);
	int i,word;

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	spin_lock_irq(&chip->reg_lock);
	for (word=0 ; word < 4; word++) {
		chip->stream[PLAYBACK]->spdif_status[word] = 
		mv88fx_snd_readl(chip,
				 MV_AUDIO_SPDIF_PLAY_CH_STATUS_LEFT_REG(word));
		for (i = 0; i < 4; i++)
			ucontrol->value.iec958.status[word + i] = 
			(chip->stream[PLAYBACK]->spdif_status[word] >>
								(i * 8)) & 0xff;

	}
	spin_unlock_irq(&chip->reg_lock);
	return 0;
}

static int mv88fx_snd_spdif_stream_put(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct mv88fx_snd_chip *chip = snd_kcontrol_chip(kcontrol);
	int i, change =0 , word;
	unsigned int val;

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
        val = 0;
	spin_lock_irq(&chip->reg_lock);
	for (word=0 ; word < 4; word++) {
		for (i = 0; i < 4; i++){
                        chip->stream[PLAYBACK]->spdif_status[word] |= 
			ucontrol->value.iec958.status[word + i] << (i * 8);
		}
		 mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_CH_STATUS_LEFT_REG(word),
			chip->stream[PLAYBACK]->spdif_status[word]);
		 
		 mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_CH_STATUS_RIGHT_REG(word),
			chip->stream[PLAYBACK]->spdif_status[word]);

	}
	
	if (chip->stream[PLAYBACK]->spdif_status[0] & IEC958_AES0_NONAUDIO) {
                chip->pcm_mode = NON_PCM;
	}
	spin_unlock_irq(&chip->reg_lock);
	return change;
}

static struct snd_kcontrol_new mv88fx_snd_spdif_stream  =
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READWRITE | 
			SNDRV_CTL_ELEM_ACCESS_INACTIVE,
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,PCM_STREAM),
	.info =		mv88fx_snd_spdif_stream_info,
	.get =		mv88fx_snd_spdif_stream_get,
	.put =		mv88fx_snd_spdif_stream_put
};


static int mv88fx_snd_spdif_default_info(struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_info *uinfo)
{
	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_IEC958;
	uinfo->count = 1;
	return 0;
}

static int mv88fx_snd_spdif_default_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	struct mv88fx_snd_chip *chip = snd_kcontrol_chip(kcontrol);
	int i,word;

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	spin_lock_irq(&chip->reg_lock);
	for (word=0 ; word < 4; word++) {
		chip->stream_defaults[PLAYBACK]->spdif_status[word] = 
		mv88fx_snd_readl(chip,
				 MV_AUDIO_SPDIF_PLAY_CH_STATUS_LEFT_REG(word));
		
                for (i = 0; i < 4; i++)
			ucontrol->value.iec958.status[word + i] = 
			(chip->stream_defaults[PLAYBACK]->spdif_status[word] >> 
								(i * 8)) & 0xff;

	}
	spin_unlock_irq(&chip->reg_lock);
	return 0;
}

static int mv88fx_snd_spdif_default_put(struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_value *ucontrol)
{
	struct mv88fx_snd_chip *chip = snd_kcontrol_chip(kcontrol);
	int i, change = 0, word;
	unsigned int val;

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
        val = 0;
	spin_lock_irq(&chip->reg_lock);
	for (word=0 ; word < 4; word++) {
		for (i = 0; i < 4; i++){
                        chip->stream_defaults[PLAYBACK]->spdif_status[word] |= 
			ucontrol->value.iec958.status[word + i] << (i * 8);
		}
		 mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_CH_STATUS_LEFT_REG(word),
			chip->stream_defaults[PLAYBACK]->spdif_status[word]);
		 mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_CH_STATUS_RIGHT_REG(word),
			chip->stream_defaults[PLAYBACK]->spdif_status[word]);
		 
		 
	}
	if (chip->stream_defaults[PLAYBACK]->spdif_status[0] & 
				IEC958_AES0_NONAUDIO) {
		chip->pcm_mode = NON_PCM;
	}
	
	spin_unlock_irq(&chip->reg_lock);
	return change;
}
static struct snd_kcontrol_new mv88fx_snd_spdif_default __devinitdata =
{
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,DEFAULT),
	.info =		mv88fx_snd_spdif_default_info,
	.get =		mv88fx_snd_spdif_default_get,
	.put =		mv88fx_snd_spdif_default_put
};




/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
void cs42l51_vol_get(unsigned char *vol_list);
void cs42l51_vol_set(unsigned char *vol_list);

unsigned char mv88fx_snd_vol[2];

static int mv88fx_snd_mixer_vol_info(struct snd_kcontrol *kcontrol, 
			       struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 39;
	return 0;
}

static int mv88fx_snd_mixer_vol_get(struct snd_kcontrol *kcontrol, 
				    struct snd_ctl_elem_value *ucontrol)
{
	cs42l51_vol_get(mv88fx_snd_vol);
	ucontrol->value.integer.value[0] = (long)mv88fx_snd_vol[0];
	ucontrol->value.integer.value[1] = (long)mv88fx_snd_vol[1];
	return 0;
}

static int mv88fx_snd_mixer_vol_put(struct snd_kcontrol *kcontrol, 
				    struct snd_ctl_elem_value *ucontrol) {
	
	mv88fx_snd_vol[0] = (unsigned char)ucontrol->value.integer.value[0];
	mv88fx_snd_vol[1] = (unsigned char)ucontrol->value.integer.value[1];
	cs42l51_vol_set(mv88fx_snd_vol);

	return 0;
}

static struct snd_kcontrol_new mv88fx_snd_dac_vol =
{
	.iface =	SNDRV_CTL_ELEM_IFACE_MIXER,
	.name =		"Playback DAC Volume",
	.info =		mv88fx_snd_mixer_vol_info,
	.get =		mv88fx_snd_mixer_vol_get,
	.put =		mv88fx_snd_mixer_vol_put
};


/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/


struct mv88fx_snd_mixer_enum {
	char 	**names;	/* enum names*/
	int	*values;	/* values to be updated*/
	int 	count;		/* number of elements */
	void*	rec;		/* field to be updated*/
};


int mv88fx_snd_mixer_enum_info (struct snd_kcontrol * kcontrol, 
			       struct snd_ctl_elem_info * uinfo)
{
	struct mv88fx_snd_mixer_enum	*mixer_enum = 
		(struct mv88fx_snd_mixer_enum*)kcontrol->private_value;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	uinfo->value.enumerated.items = mixer_enum->count;

	if (uinfo->value.enumerated.item >= uinfo->value.enumerated.items)
		uinfo->value.enumerated.item =
		    uinfo->value.enumerated.items - 1;

	strcpy(uinfo->value.enumerated.name,
	       mixer_enum->names[uinfo->value.enumerated.item]);

	return 0;
}
int mv88fx_snd_mixer_enum_get (struct snd_kcontrol * kcontrol, 
			      struct snd_ctl_elem_value * ucontrol)
{
	struct mv88fx_snd_mixer_enum	*mixer_enum = 
		(struct mv88fx_snd_mixer_enum*)kcontrol->private_value;
	int i;
	unsigned int val;

	val = *(unsigned int*)mixer_enum->rec;
	
	for (i=0 ; i<mixer_enum->count ; i++) {
		
		if (val ==  (unsigned int)mixer_enum->values[i]) {
			ucontrol->value.enumerated.item[0] = i;
			break;
		}
	}
	
	return 0;
}

int mv88fx_snd_mixer_enum_put (struct snd_kcontrol * kcontrol, 
			      struct snd_ctl_elem_value * ucontrol)
{
	unsigned int val,*rec;
	struct mv88fx_snd_mixer_enum	*mixer_enum = 
		(struct mv88fx_snd_mixer_enum*)kcontrol->private_value;
	int i;

	rec = (unsigned int*)mixer_enum->rec;
	val = ucontrol->value.enumerated.item[0];

	if (val < 0)
		val = 0;
	if (val > mixer_enum->count)
		val = mixer_enum->count;

	for (i=0 ; i<mixer_enum->count ; i++) {

		if (val ==  i) {
			*rec = (unsigned int)mixer_enum->values[i];
			break;
		}
	}
	
	return 0;
}



#define MV88FX_PCM_MIXER_ENUM(xname, xindex, value)	\
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .name = xname, \
  .index = xindex, \
  .info = mv88fx_snd_mixer_enum_info, \
  .get = mv88fx_snd_mixer_enum_get, \
  .put = mv88fx_snd_mixer_enum_put, \
  .private_value = (unsigned long)value,	\
}

char *playback_src_mixer_names[] = {"SPDIF","I2S", "SPDIF And I2S"};
int playback_src_mixer_values[] = { SPDIF, I2S, (SPDIF|I2S)};

struct mv88fx_snd_mixer_enum playback_src_mixer	= 
{
	.names	= playback_src_mixer_names,
	.values	= playback_src_mixer_values,
	.count	= 3,
};

char *playback_mono_mixer_names[] = {"Mono Both","Mono Left", "Mono Right"};
int playback_mono_mixer_values[] = { MONO_BOTH, MONO_LEFT, MONO_RIGHT};

struct mv88fx_snd_mixer_enum playback_mono_mixer	= 
{
	.names	= playback_mono_mixer_names,
	.values	= playback_mono_mixer_values,
	.count	= 3,
};




char *capture_src_mixer_names[] = {"SPDIF","I2S"};
int capture_src_mixer_values[] = { SPDIF, I2S};

struct mv88fx_snd_mixer_enum capture_src_mixer	= 
{
	.names	= capture_src_mixer_names,
	.values	= capture_src_mixer_values,
	.count	= 2,
};

char *capture_mono_mixer_names[] = {"Mono Left", "Mono Right"};
int capture_mono_mixer_values[] = { MONO_LEFT, MONO_RIGHT};

struct mv88fx_snd_mixer_enum capture_mono_mixer	= 
{
	.names	= capture_mono_mixer_names,
	.values	= capture_mono_mixer_values,
	.count	= 2,
};


static struct snd_kcontrol_new mv88fx_snd_mixers[] = {
	MV88FX_PCM_MIXER_ENUM("Playback output type", 0,
				    &playback_src_mixer), 

	MV88FX_PCM_MIXER_ENUM("Playback mono type", 0,
				    &playback_mono_mixer), 

	MV88FX_PCM_MIXER_ENUM("Capture input Type", 0,
				    &capture_src_mixer),
	
	MV88FX_PCM_MIXER_ENUM("Capture mono type", 0,
				    &capture_mono_mixer), 

};



static int
mv88fx_snd_ctrl_new(struct snd_card *card)
{
	int err=0, idx;

	playback_src_mixer.rec = &chip->stream_defaults[PLAYBACK]->dig_mode;
	playback_mono_mixer.rec = &chip->stream_defaults[PLAYBACK]->mono_mode;
	
	capture_src_mixer.rec = &chip->stream_defaults[CAPTURE]->dig_mode;
	capture_mono_mixer.rec = &chip->stream_defaults[CAPTURE]->mono_mode;

	for (idx = 0; idx < ARRAY_SIZE(mv88fx_snd_mixers); idx++) {
		err = snd_ctl_add(card, snd_ctl_new1(&mv88fx_snd_mixers[idx],
						     chip));
		if (err < 0)
			return err;
	}

	
	err = snd_ctl_add(card, snd_ctl_new1(&mv88fx_snd_dac_vol,
					     chip));
	if (err < 0)
		return err;
		
	err = snd_ctl_add(card, snd_ctl_new1(&mv88fx_snd_spdif_mask,
					     chip));
	if (err < 0)
		return err;

	err = snd_ctl_add(card, snd_ctl_new1(&mv88fx_snd_spdif_default,
					     chip));
	if (err < 0)
		return err;

	err = snd_ctl_add(card, snd_ctl_new1(&mv88fx_snd_spdif_stream,
					     chip));
	if (err < 0)
		return err;
	
	

	return err;
}


/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/



static struct snd_pcm_hardware mv88fx_snd_capture_hw =
{
	.info =			(SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_MMAP |
				 SNDRV_PCM_INFO_MMAP_VALID |
				 SNDRV_PCM_INFO_BLOCK_TRANSFER |
				 SNDRV_PCM_INFO_PAUSE ), 
	.formats =		(SNDRV_PCM_FMTBIT_S16_LE |
				 SNDRV_PCM_FMTBIT_S24_LE |
				 SNDRV_PCM_FMTBIT_S32_LE),
	.rates =		(SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 |
				 SNDRV_PCM_RATE_96000),
	.rate_min =		44100,
	.rate_max =		96000,
	.channels_min =		1,
	.channels_max =		2,
	.buffer_bytes_max =	(16*1024*1024),
	.period_bytes_min =	MV88FX_SND_MIN_PERIOD_BYTES,
	.period_bytes_max =	MV88FX_SND_MAX_PERIOD_BYTES,
	.periods_min =		MV88FX_SND_MIN_PERIODS,
	.periods_max =		MV88FX_SND_MAX_PERIODS,
	.fifo_size =		0,

};



static int
mv88fx_snd_capture_open(struct snd_pcm_substream * substream)
{
	int err;

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	chip->stream_defaults[CAPTURE]->substream = substream;
	chip->stream_defaults[CAPTURE]->direction = CAPTURE;
	substream->private_data = chip->stream_defaults[CAPTURE];
	substream->runtime->hw = mv88fx_snd_capture_hw;

	if (chip->stream_defaults[CAPTURE]->dig_mode & SPDIF) {
		substream->runtime->hw.formats &= ~SNDRV_PCM_FMTBIT_S32_LE;
	}
	
	/* check if playback is already running with specific rate */
	if (chip->stream[PLAYBACK]->rate) {
		switch 	(chip->stream[PLAYBACK]->rate) {
			case 44100:
				substream->runtime->hw.rates = SNDRV_PCM_RATE_44100;
				break;
			case 48000:
				substream->runtime->hw.rates = SNDRV_PCM_RATE_48000;
				break;
			case 96000:
				substream->runtime->hw.rates = SNDRV_PCM_RATE_96000;
				break;

		}
	}
	
	
	err = snd_pcm_hw_constraint_minmax(substream->runtime,
					   SNDRV_PCM_HW_PARAM_BUFFER_BYTES,
					   chip->burst * 2, 
					   AUDIO_REG_TO_SIZE(APBBCR_SIZE_MAX));
        if (err < 0)
                return err;
	
	err = snd_pcm_hw_constraint_step(substream->runtime, 0,
					 SNDRV_PCM_HW_PARAM_BUFFER_BYTES,
					 chip->burst); 
        if (err < 0)
                return err;
	
	err = snd_pcm_hw_constraint_step(substream->runtime, 0, 
					 SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
					 chip->burst);
        if (err < 0)
                return err;

	err = snd_pcm_hw_constraint_minmax(substream->runtime,
					   SNDRV_PCM_HW_PARAM_PERIODS,
					   MV88FX_SND_MIN_PERIODS, 
					   MV88FX_SND_MAX_PERIODS);
        if (err < 0)
                return err;
	
        err = snd_pcm_hw_constraint_integer(substream->runtime, 
					    SNDRV_PCM_HW_PARAM_PERIODS);
        if (err < 0)
                return err;
        
	return 0;

}

static int
mv88fx_snd_capture_close(struct snd_pcm_substream * substream)
{
	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	chip->stream_defaults[CAPTURE]->substream = NULL;
	memset(chip->stream[CAPTURE] , 
		0, 
		sizeof(struct mv88fx_snd_stream));

	return 0;
}
static int
mv88fx_snd_capture_hw_params(struct snd_pcm_substream * substream,
					struct snd_pcm_hw_params* params)
{
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); int err = 0; 
	
	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	
        err = devdma_hw_alloc(audio_stream->dev, substream,
			      params_buffer_bytes(params));
	return err;
}

static int
mv88fx_snd_capture_hw_free(struct snd_pcm_substream * substream)
{
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); 

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	/*
	 * Clear out the DMA and any allocated buffers.
	 */
	devdma_hw_free(audio_stream->dev, substream);
	return 0;
	
}

static int
mv88fx_snd_capture_prepare(struct snd_pcm_substream * substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); 


	audio_stream->rate = runtime->rate;
        audio_stream->stereo= (runtime->channels == 1) ? 0 : 1;

	if (runtime->format == SNDRV_PCM_FORMAT_S16_LE) {
		audio_stream->format = SAMPLE_16IN16;
	} else if (runtime->format == SNDRV_PCM_FORMAT_S24_LE) {
		audio_stream->format = SAMPLE_24IN32;
	} else if (runtime->format == SNDRV_PCM_FORMAT_S32_LE) {
		audio_stream->format = SAMPLE_32IN32;
	} else {
		mv88fx_snd_debug("Requested format %d is not supported\n", 
			   runtime->format);
		return -1;
	}

	/* buffer and period sizes in frame */
	audio_stream->dma_addr = runtime->dma_addr;
	audio_stream->dma_size = 
		frames_to_bytes(runtime, runtime->buffer_size);
	audio_stream->period_size = 
		frames_to_bytes(runtime, runtime->period_size);

	memcpy(chip->stream[CAPTURE], 
	       chip->stream_defaults[CAPTURE],
	       sizeof(struct mv88fx_snd_stream));
	return mv88fx_snd_hw_capture_set(chip);
}

static int
mv88fx_snd_capture_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); 
	int result = 0;

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	
	spin_lock(chip->reg_lock);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		/* FIXME: should check if busy before */

		/* make sure the dma in pause state*/
		mv88fx_snd_bitset(chip, MV_AUDIO_RECORD_CTRL_REG,
				  ARCR_RECORD_PAUSE_MASK);

		/* enable interrupt */
		mv88fx_snd_bitset(chip, MV_AUDIO_INT_MASK_REG, 
				  AICR_RECORD_BYTES_INT);


		/* enable dma */
		if (audio_stream->dig_mode & I2S)
			mv88fx_snd_bitset(chip, MV_AUDIO_RECORD_CTRL_REG,
					  ARCR_RECORD_I2S_EN_MASK);

		if (audio_stream->dig_mode & SPDIF)
			mv88fx_snd_bitset(chip, MV_AUDIO_RECORD_CTRL_REG,
					  ARCR_RECORD_SPDIF_EN_MASK);

		/* start dma */
		mv88fx_snd_bitreset(chip, MV_AUDIO_RECORD_CTRL_REG,
				  ARCR_RECORD_PAUSE_MASK);
		
		
		break;
	case SNDRV_PCM_TRIGGER_STOP:

		/* make sure the dma in pause state*/
		mv88fx_snd_bitset(chip, MV_AUDIO_RECORD_CTRL_REG,
				  ARCR_RECORD_PAUSE_MASK);
		
                /* disable interrupt */
		mv88fx_snd_bitreset(chip, MV_AUDIO_INT_MASK_REG, 
				  AICR_RECORD_BYTES_INT);

		
		/* always stop both I2S and SPDIF*/
		mv88fx_snd_bitreset(chip, MV_AUDIO_RECORD_CTRL_REG,
				    (ARCR_RECORD_I2S_EN_MASK | 
				     ARCR_RECORD_SPDIF_EN_MASK));
		
                /* FIXME: should check if busy after */
		
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		mv88fx_snd_bitset(chip, MV_AUDIO_RECORD_CTRL_REG,
				  ARCR_RECORD_PAUSE_MASK);
		break;
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		mv88fx_snd_bitreset(chip, MV_AUDIO_RECORD_CTRL_REG,
				    ARCR_RECORD_PAUSE_MASK);

		break;
	default:
		result = -EINVAL;
		break;
	}
	spin_unlock(&chip->reg_lock);
	return result;
}


static snd_pcm_uframes_t 
mv88fx_snd_capture_pointer(struct snd_pcm_substream * substream)
{
	return bytes_to_frames(substream->runtime, 
			       (ssize_t)mv88fx_snd_readl(chip, 
		       		MV_AUDIO_RECORD_BUF_BYTE_CNTR_REG));
}

int 
mv88fx_snd_capture_mmap(struct snd_pcm_substream *substream, 
			 struct vm_area_struct *vma)
{
	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	return devdma_mmap(NULL, substream, vma);	
	
}

static struct snd_pcm_ops mv88fx_snd_capture_ops = {
	.open			= mv88fx_snd_capture_open,
	.close			= mv88fx_snd_capture_close,
	.ioctl			= snd_pcm_lib_ioctl,
	.hw_params	        = mv88fx_snd_capture_hw_params,
	.hw_free	        = mv88fx_snd_capture_hw_free,
	.prepare		= mv88fx_snd_capture_prepare,
	.trigger		= mv88fx_snd_capture_trigger,
	.pointer		= mv88fx_snd_capture_pointer,
	.mmap 			= mv88fx_snd_capture_mmap,
};

/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/



struct snd_pcm_hardware mv88fx_snd_playback_hw =
{
	.info =			(SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_MMAP |
				 SNDRV_PCM_INFO_MMAP_VALID |
				 SNDRV_PCM_INFO_BLOCK_TRANSFER |
				 SNDRV_PCM_INFO_PAUSE ), 
	.formats =		(SNDRV_PCM_FMTBIT_S16_LE |
				 SNDRV_PCM_FMTBIT_S24_LE |
				 SNDRV_PCM_FMTBIT_S32_LE),
	.rates =		(SNDRV_PCM_RATE_44100 |
				 SNDRV_PCM_RATE_48000 |
				 SNDRV_PCM_RATE_96000),
	.rate_min =		44100,
	.rate_max =		96000,
	.channels_min =		1,
	.channels_max =		2,
	.buffer_bytes_max =	(16*1024*1024),
	.period_bytes_min =	MV88FX_SND_MIN_PERIOD_BYTES,
	.period_bytes_max =	MV88FX_SND_MAX_PERIOD_BYTES,
	.periods_min =		MV88FX_SND_MIN_PERIODS,
	.periods_max =		MV88FX_SND_MAX_PERIODS,
	.fifo_size =		0,

};

static int
mv88fx_snd_playback_open(struct snd_pcm_substream * substream)
{
	int err = 0; 

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	chip->stream_defaults[PLAYBACK]->substream = substream;
	chip->stream_defaults[PLAYBACK]->direction = PLAYBACK;
	substream->private_data = chip->stream_defaults[PLAYBACK];
	substream->runtime->hw = mv88fx_snd_playback_hw;

	if (chip->stream_defaults[PLAYBACK]->dig_mode & SPDIF) {
		substream->runtime->hw.formats &= ~SNDRV_PCM_FMTBIT_S32_LE;
	}

	/* check if capture is already running with specific rate */
	if (chip->stream[CAPTURE]->rate) {
		switch 	(chip->stream[CAPTURE]->rate) {
			case 44100:
				substream->runtime->hw.rates = SNDRV_PCM_RATE_44100;
				break;
			case 48000:
				substream->runtime->hw.rates = SNDRV_PCM_RATE_48000;
				break;
			case 96000:
				substream->runtime->hw.rates = SNDRV_PCM_RATE_96000;
				break;

		}
	}

	err = snd_pcm_hw_constraint_minmax(substream->runtime,
					   SNDRV_PCM_HW_PARAM_BUFFER_BYTES,
					   chip->burst * 2, 
					   AUDIO_REG_TO_SIZE(APBBCR_SIZE_MAX));
        if (err < 0)
                return err;
	
	err = snd_pcm_hw_constraint_step(substream->runtime, 0,
					 SNDRV_PCM_HW_PARAM_BUFFER_BYTES,
					 chip->burst); 
        if (err < 0)
                return err;
	
	err = snd_pcm_hw_constraint_step(substream->runtime, 0, 
					 SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
					 chip->burst);
        if (err < 0)
                return err;

	err = snd_pcm_hw_constraint_minmax(substream->runtime,
					   SNDRV_PCM_HW_PARAM_PERIODS,
					   MV88FX_SND_MIN_PERIODS, 
					   MV88FX_SND_MAX_PERIODS);
        if (err < 0)
                return err;
	
        err = snd_pcm_hw_constraint_integer(substream->runtime, 
					    SNDRV_PCM_HW_PARAM_PERIODS);

        if (err < 0)
                return err;
        
	return 0;

}

static int
mv88fx_snd_playback_close(struct snd_pcm_substream * substream)
{
	int i;
	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	chip->stream_defaults[PLAYBACK]->substream = NULL;

	chip->pcm_mode = PCM;

	for (i=0; i<4 ; i++) {
		chip->stream_defaults[PLAYBACK]->spdif_status[i] = 0;
		chip->stream[PLAYBACK]->spdif_status[i] = 0;

		 mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_CH_STATUS_LEFT_REG(i),
			chip->stream_defaults[PLAYBACK]->spdif_status[i]);
		 mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_CH_STATUS_RIGHT_REG(i),
			chip->stream_defaults[PLAYBACK]->spdif_status[i]);

	}

	memset(chip->stream[PLAYBACK] , 
		0, 
		sizeof(struct mv88fx_snd_stream));

	
	return 0;
}
static int
mv88fx_snd_playback_hw_params(struct snd_pcm_substream * substream,
					struct snd_pcm_hw_params* params)
{
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); int err = 0; 
	
	
        err = devdma_hw_alloc(audio_stream->dev, substream,
			      params_buffer_bytes(params));

	//mv88fx_snd_debug("=>%s addr=0x%x size=0x%x\n",__FUNCTION__,
	//	   substream->runtime->dma_addr,
	//	   frames_to_bytes(substream->runtime, 
	//			   substream->runtime->buffer_size));
	

	return err;
}

static int
mv88fx_snd_playback_hw_free(struct snd_pcm_substream * substream)
{
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); 

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	/*
	 * Clear out the DMA and any allocated buffers.
	 */
	devdma_hw_free(audio_stream->dev, substream);
	return 0;
	
}

static int
mv88fx_snd_playback_prepare(struct snd_pcm_substream * substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); 

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
        if ((audio_stream->dig_mode == I2S) && 
	    (chip->pcm_mode == NON_PCM))
		return -1;

	audio_stream->rate = runtime->rate;
        audio_stream->stereo= (runtime->channels == 1) ? 0 : 1;

	if (runtime->format == SNDRV_PCM_FORMAT_S16_LE) {
		audio_stream->format = SAMPLE_16IN16;
	} else if (runtime->format == SNDRV_PCM_FORMAT_S24_LE) {
		audio_stream->format = SAMPLE_24IN32;
	} else if (runtime->format == SNDRV_PCM_FORMAT_S32_LE) {
		audio_stream->format = SAMPLE_32IN32;
	} else {
		mv88fx_snd_debug("Requested format %d is not supported\n", 
			   runtime->format);
		return -1;
	}

	/* buffer and period sizes in frame */
	audio_stream->dma_addr = runtime->dma_addr;
	audio_stream->dma_size = 
		frames_to_bytes(runtime, runtime->buffer_size);
	audio_stream->period_size = 
		frames_to_bytes(runtime, runtime->period_size);

	memcpy(chip->stream[PLAYBACK], 
	       chip->stream_defaults[PLAYBACK],
	       sizeof(struct mv88fx_snd_stream));
	
	return mv88fx_snd_hw_playback_set(chip);
}

static int
mv88fx_snd_playback_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct mv88fx_snd_stream *audio_stream = 
		snd_pcm_substream_chip(substream); 
	int result = 0;

	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	
	spin_lock(chip->reg_lock);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		/* enable interrupt */
		mv88fx_snd_bitset(chip, MV_AUDIO_INT_MASK_REG, 
				  AICR_PLAY_BYTES_INT);

		/* make sure the dma in pause state*/
		mv88fx_snd_bitset(chip, MV_AUDIO_PLAYBACK_CTRL_REG,
				  APCR_PLAY_PAUSE_MASK);

		/* enable dma */
                if ((audio_stream->dig_mode & I2S) && 
		    (chip->pcm_mode == PCM))
			mv88fx_snd_bitset(chip, MV_AUDIO_PLAYBACK_CTRL_REG,
					  APCR_PLAY_I2S_ENABLE_MASK);
		
		if (audio_stream->dig_mode & SPDIF)
			mv88fx_snd_bitset(chip, MV_AUDIO_PLAYBACK_CTRL_REG,
					  APCR_PLAY_SPDIF_ENABLE_MASK);

		/* start dma */
		mv88fx_snd_bitreset(chip, MV_AUDIO_PLAYBACK_CTRL_REG,
				    APCR_PLAY_PAUSE_MASK);
		
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		
                /* disable interrupt */
		mv88fx_snd_bitreset(chip, MV_AUDIO_INT_MASK_REG, 
				  AICR_PLAY_BYTES_INT);


		/* make sure the dma in pause state*/
		mv88fx_snd_bitset(chip, MV_AUDIO_PLAYBACK_CTRL_REG,
				  APCR_PLAY_PAUSE_MASK);
		
		/* always stop both I2S and SPDIF*/
		mv88fx_snd_bitreset(chip, MV_AUDIO_PLAYBACK_CTRL_REG,
				    (APCR_PLAY_I2S_ENABLE_MASK | 
				     APCR_PLAY_SPDIF_ENABLE_MASK));

		/* check if busy twice*/
		while (mv88fx_snd_readl(chip, MV_AUDIO_PLAYBACK_CTRL_REG) & 
		       APCR_PLAY_BUSY_MASK);
		
		while (mv88fx_snd_readl(chip, MV_AUDIO_PLAYBACK_CTRL_REG) & 
		       APCR_PLAY_BUSY_MASK);
		
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		mv88fx_snd_bitset(chip, MV_AUDIO_PLAYBACK_CTRL_REG,
				  APCR_PLAY_PAUSE_MASK);
		break;
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		mv88fx_snd_bitreset(chip, MV_AUDIO_PLAYBACK_CTRL_REG,
				    APCR_PLAY_PAUSE_MASK);

		break;
	default:
		result = -EINVAL;
		break;
	}
	spin_unlock(&chip->reg_lock);
	return result;
}


static snd_pcm_uframes_t 
mv88fx_snd_playback_pointer(struct snd_pcm_substream * substream)
{
	return bytes_to_frames(substream->runtime, 
			       (ssize_t)mv88fx_snd_readl(chip, 
		       		MV_AUDIO_PLAYBACK_BUFF_BYTE_CNTR_REG));
}

int 
mv88fx_snd_playback_mmap(struct snd_pcm_substream *substream, 
			 struct vm_area_struct *vma)
{
	//mv88fx_snd_debug("=>%s\n",__FUNCTION__);
	return devdma_mmap(NULL, substream, vma);	
	
}


static struct snd_pcm_ops mv88fx_snd_playback_ops = {
	.open			= mv88fx_snd_playback_open,
	.close			= mv88fx_snd_playback_close,
	.ioctl			= snd_pcm_lib_ioctl,
	.hw_params	        = mv88fx_snd_playback_hw_params,
	.hw_free	        = mv88fx_snd_playback_hw_free,
	.prepare		= mv88fx_snd_playback_prepare,
	.trigger		= mv88fx_snd_playback_trigger,
	.pointer		= mv88fx_snd_playback_pointer,
	.mmap 			= mv88fx_snd_playback_mmap,
};


/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/


static int __init 
mv88fx_snd_pcm_new(struct snd_card *card)
{
	struct snd_pcm *pcm;
	int err, i;

	mv88fx_snd_debug("=>%s card->dev=0x%x\n",__FUNCTION__, 
		   (unsigned int)card->dev);
	
	if ((err = snd_pcm_new(card, "Marvell mv88fx_snd IEC958 and I2S", 
			       0, 1, 1, &pcm)) < 0) 
		return err;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, 
			&mv88fx_snd_playback_ops);
	
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, 
			&mv88fx_snd_capture_ops);
	
	chip->stream_defaults[PLAYBACK]->dig_mode = (SPDIF | I2S);
	chip->stream_defaults[PLAYBACK]->mono_mode = MONO_BOTH;
	chip->pcm_mode = PCM;
	chip->stream_defaults[PLAYBACK]->stat_mem = 0;
	chip->stream_defaults[PLAYBACK]->clock_src = DCO_CLOCK;

	chip->stream_defaults[CAPTURE]->dig_mode = I2S;
	chip->stream_defaults[CAPTURE]->mono_mode = MONO_LEFT;
	chip->pcm_mode = PCM;
	chip->stream_defaults[CAPTURE]->stat_mem = 0;
	chip->stream_defaults[CAPTURE]->clock_src = DCO_CLOCK;

	for (i=0; i<4 ; i++) {
		chip->stream_defaults[PLAYBACK]->spdif_status[i] = 0;
		chip->stream[PLAYBACK]->spdif_status[i] = 0;
		
		mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_CH_STATUS_LEFT_REG(i),
			chip->stream_defaults[PLAYBACK]->spdif_status[i]);
		mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_CH_STATUS_RIGHT_REG(i),
			chip->stream_defaults[PLAYBACK]->spdif_status[i]);
		mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_USR_BITS_LEFT_REG(i),
			0);
		mv88fx_snd_writel(chip, 
			MV_AUDIO_SPDIF_PLAY_USR_BITS_RIGHT_REG(i),
			0);
		
	}
	

	pcm->private_data = chip;
	pcm->info_flags = 0;
        strcpy(pcm->name, "Marvell mv88fx_snd IEC958 and I2S");

	return 0;
}

static irqreturn_t
mv88fx_snd_interrupt(int irq, void *dev_id)
{
	struct mv88fx_snd_chip *chip = dev_id;
	struct mv88fx_snd_stream *play_stream =
		chip->stream_defaults[PLAYBACK];
	struct mv88fx_snd_stream *capture_stream =
		chip->stream_defaults[CAPTURE];
	
			
	unsigned int status, mask;

	spin_lock(&chip->reg_lock);
	
	/* read the active interrupt */
	mask = mv88fx_snd_readl(chip, MV_AUDIO_INT_MASK_REG);
	status = mv88fx_snd_readl(chip, MV_AUDIO_INT_CAUSE_REG) & mask;

	do {
	

		if (status & ~(AICR_RECORD_BYTES_INT|AICR_PLAY_BYTES_INT)) {
	
	                spin_unlock(&chip->reg_lock);
			snd_BUG(); /* FIXME: should enable error interrupts*/
			return IRQ_NONE;
		}
	
		/* acknowledge interrupt */
		mv88fx_snd_writel(chip, MV_AUDIO_INT_CAUSE_REG, status);
	
		/* This is record event */
		if (status & AICR_RECORD_BYTES_INT) {
	
			snd_pcm_period_elapsed(capture_stream->substream);
			//printk("int:capture_stream=0x%x rec_count=0x%x\n",(unsigned int)capture_stream, mv88fx_snd_readl(chip, 
			  //     		MV_AUDIO_RECORD_BUF_BYTE_CNTR_REG));
		}
		/* This is play event */
		if (status & AICR_PLAY_BYTES_INT) {
			
			snd_pcm_period_elapsed(play_stream->substream);
			//printk("int:play_stream=0x%x rec_count=0x%x\n",(unsigned int)play_stream, mv88fx_snd_readl(chip, 
			  //     		MV_AUDIO_PLAYBACK_BUFF_BYTE_CNTR_REG));
		
		}
	
		/* read the active interrupt */
		mask = mv88fx_snd_readl(chip, MV_AUDIO_INT_MASK_REG);
		status = mv88fx_snd_readl(chip, MV_AUDIO_INT_CAUSE_REG) & mask;

	} while(status);

	spin_unlock(&chip->reg_lock);

	return IRQ_HANDLED;
}

static void 
mv88fx_snd_free(struct snd_card *card)
{
	struct mv88fx_snd_chip *chip = card->private_data;

	mv88fx_snd_debug("=>%s chip=0x%x\n",__FUNCTION__,(unsigned int)chip);
	
	/* free irq */
	free_irq(chip->irq, (void *)chip);

	mv88fx_snd_debug(KERN_ERR "chip->res =0x%x\n", (unsigned int)chip->res);

	if (chip->base)
		iounmap(chip->base);
	
	if (chip->res) 
		release_resource(chip->res);
	
	chip->res = NULL;
	
		
	/* Free memory allocated for streems */
	if (chip->stream_defaults[PLAYBACK]) {
		kfree(chip->stream_defaults[PLAYBACK]);
		chip->stream_defaults[PLAYBACK] = NULL;
	}
	if (chip->stream[PLAYBACK]) {
		kfree(chip->stream[PLAYBACK]);
		chip->stream[PLAYBACK] = NULL;
	}	
	if (chip->stream_defaults[CAPTURE]) {
		kfree(chip->stream_defaults[CAPTURE]);
		chip->stream_defaults[CAPTURE] = NULL;
	}
	if (chip->stream[CAPTURE]) {
		kfree(chip->stream[CAPTURE]);
		chip->stream[CAPTURE] = NULL;
	}

	if (chip->stream_defaults[PLAYBACK]->area)
		dma_free_coherent(chip->dev, 
				MV88FX_SND_MAX_PERIODS * MV88FX_SND_MAX_PERIOD_BYTES, 
				chip->stream_defaults[PLAYBACK]->area, 
				chip->stream_defaults[PLAYBACK]->addr);	

	if (chip->stream_defaults[CAPTURE]->area)
		dma_free_coherent(chip->dev, 
				MV88FX_SND_MAX_PERIODS * MV88FX_SND_MAX_PERIOD_BYTES, 
				chip->stream_defaults[CAPTURE]->area, 
				chip->stream_defaults[CAPTURE]->addr);	


	chip = NULL;
}

static int mv88fx_snd_probe(struct platform_device *dev)
{
	int err = 0, irq = NO_IRQ;	
	struct snd_card *card = NULL;
	struct resource *r = NULL;	
	static struct snd_device_ops ops = {
		.dev_free =	NULL,
	};
	
	mv88fx_snd_debug("=>%s", __FUNCTION__);


        card = snd_card_new(-1, "mv88fx_snd", THIS_MODULE, 
			    sizeof(struct mv88fx_snd_chip));

	if (card == NULL) {
		mv88fx_snd_debug(KERN_ERR "snd_card_new failed\n");
		return -ENOMEM;
	}

        chip = card->private_data;
	card->dev = &dev->dev;
        chip->dev = &dev->dev;
	chip->mem_array = (_audio_mem_info*)dev->dev.platform_data;

	card->private_free = mv88fx_snd_free;

	r = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (!r) {
		mv88fx_snd_debug(KERN_ERR "platform_get_resource failed\n");
		err = -ENXIO;
		goto error;
	}

	
        mv88fx_snd_debug(KERN_ERR "chip->res =0x%x\n", (unsigned int)chip->res);
	
	r = request_mem_region(r->start, SZ_16K, DRIVER_NAME);
	if (!r){
		mv88fx_snd_debug(KERN_ERR "request_mem_region failed\n");
		err = -EBUSY;
		goto error;
	} 
	chip->res = r;

	irq = platform_get_irq(dev, 0);
	if (irq == NO_IRQ){
		mv88fx_snd_debug(KERN_ERR "platform_get_irq failed\n");
		err = -ENXIO;
		goto error;
	}

        
	mv88fx_snd_debug("%s card = 0x%x card->dev 0x%x\n",__FUNCTION__,
						(unsigned int)card,
						(unsigned int)card->dev);
        chip->base = ioremap(r->start, SZ_16K);

	if (!chip->base) {
		mv88fx_snd_debug(KERN_ERR "ioremap failed\n");
		err = -ENOMEM;
                goto error;
	}

	mv88fx_snd_debug("%s: chip->base=0x%x r->start0x%x\n",__FUNCTION__,
		   (unsigned int)chip->base,r->start);

	chip->pdata = (struct mv88fx_snd_platform_data*) dev->dev.platform_data;
		
        strncpy(card->driver, dev->dev.driver->name, 
		sizeof(card->driver));	
	
        
	/* Allocate memory for our device */
	chip->stream_defaults[PLAYBACK] = 
		kzalloc(sizeof(struct mv88fx_snd_stream), GFP_KERNEL);
	
        if (chip->stream_defaults[PLAYBACK] == NULL) {
		mv88fx_snd_debug(KERN_ERR "kzalloc failed for default playback\n");
		err = -ENOMEM;
		goto error;
	}
	chip->stream_defaults[PLAYBACK]->direction = PLAYBACK;
	chip->stream_defaults[PLAYBACK]->dev = card->dev;

	chip->stream[PLAYBACK] = kzalloc(sizeof(struct mv88fx_snd_stream),
					 GFP_KERNEL);

	if (chip->stream[PLAYBACK] == NULL) {
		mv88fx_snd_debug(KERN_ERR "kzalloc failed for runtime playback\n");
		err = -ENOMEM;
		goto error;
	}
	
	
	chip->stream_defaults[CAPTURE] = 
		kzalloc(sizeof(struct mv88fx_snd_stream), GFP_KERNEL);
	
	if (chip->stream_defaults[CAPTURE] == NULL) {
		mv88fx_snd_debug(KERN_ERR "kzalloc failed for capture\n");
		err = -ENOMEM;
		goto error;
	}
	chip->stream_defaults[CAPTURE]->direction = CAPTURE;
        chip->stream_defaults[CAPTURE]->dev = card->dev;

	chip->stream[CAPTURE] = kzalloc(sizeof(struct mv88fx_snd_stream),
					 GFP_KERNEL);

	if (chip->stream[CAPTURE] == NULL) {
		mv88fx_snd_debug(KERN_ERR "kzalloc failed for runtime capture\n");
		err = -ENOMEM;
		goto error;
	}
	
	chip->irq = irq;
	chip->stream_defaults[PLAYBACK]->area = 
		dma_alloc_coherent(chip->dev,
				MV88FX_SND_MAX_PERIODS * MV88FX_SND_MAX_PERIOD_BYTES, 
				&chip->stream_defaults[PLAYBACK]->addr, 
				GFP_KERNEL);

	if (!chip->stream_defaults[PLAYBACK]->area) {
		mv88fx_snd_debug(KERN_ERR "dma_alloc_coherent failed for playback buffer\n");
		err = -ENOMEM;
		goto error;
	}

	if (0 == test_memory(chip, 
			(unsigned int)chip->stream_defaults[PLAYBACK]->addr, 
			(unsigned int)MV88FX_SND_MAX_PERIODS * MV88FX_SND_MAX_PERIOD_BYTES)) {

		mv88fx_snd_debug(KERN_ERR "error: playback buffer not in one memory window\n");
		err = -ENOMEM;
		goto error;

	}
	
	chip->stream_defaults[CAPTURE]->area = 
		dma_alloc_coherent(chip->dev,	
				MV88FX_SND_MAX_PERIODS * MV88FX_SND_MAX_PERIOD_BYTES, 
				&chip->stream_defaults[CAPTURE]->addr, 
				GFP_KERNEL);

	if (!chip->stream_defaults[CAPTURE]->area) {
		mv88fx_snd_debug(KERN_ERR "dma_alloc_coherent failed for capture buffer\n");
		err = -ENOMEM;
		goto error;
	}

	if (0 == test_memory(chip, 
			(unsigned int)chip->stream_defaults[CAPTURE]->addr, 
			(unsigned int)MV88FX_SND_MAX_PERIODS * MV88FX_SND_MAX_PERIOD_BYTES)) {

		mv88fx_snd_debug(KERN_ERR "error: playback buffer not in one memory window\n");
		err = -ENOMEM;
		goto error;

	}

	if (request_irq(chip->irq, mv88fx_snd_interrupt, 0, DRIVER_NAME, 
			(void *)chip)) {
		
		mv88fx_snd_debug("Unable to grab IRQ %d\n", chip->irq);
		err = -ENOMEM;
		goto error;
	}

	chip->ch_stat_valid = 1; 
	chip->burst = 128;	
        chip->loopback = 0; 
        chip->dco_ctrl_offst = 0x800;
	
	err = mv88fx_snd_hw_init(chip);

	if (err != 0) {
		mv88fx_snd_debug(KERN_ERR "mv88fx_snd_hw_init failed\n");
		err = -ENOMEM;
		goto error;
	}
	
	/* Set default values */
	if ((err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops)) < 0) {
		mv88fx_snd_debug(KERN_ERR "Creating chip device failed.\n");
		err = -ENOMEM;
		goto error;
	}
	
	/* create pcm devices */
	if ((err = mv88fx_snd_pcm_new(card)) < 0) {
		mv88fx_snd_debug(KERN_ERR "Creating PCM device failed.\n");
		err = -ENOMEM;
		goto error;
	}
	/* create controll interfaces & switches */
	if ((err = mv88fx_snd_ctrl_new(card)) < 0) {
		mv88fx_snd_debug(KERN_ERR "Creating non-PCM device failed.\n");
		err = -ENOMEM;
		goto error;
	}
	
	strcpy(card->driver, "mv88fx_snd");
	strcpy(card->shortname, "Marvell mv88fx_snd");
	sprintf(card->longname, "Marvell mv88fx_snd ALSA driver");

	
	if ((err = snd_card_register(card)) < 0) {
		mv88fx_snd_debug("Card registeration failed.\n");
		err = -ENOMEM;
		goto error;
	}

	if (dma_set_mask(&dev->dev, 0xFFFFFFUL) < 0)
		goto error;
	
	platform_set_drvdata(dev, card);
	return 0;
error:
	if (card) 
                snd_card_free(card);
	platform_set_drvdata(dev, NULL);
	return err;
}

static int mv88fx_snd_remove(struct platform_device *dev)
{
	struct snd_card *card = platform_get_drvdata(dev);
	
	mv88fx_snd_debug("=>%s", __FUNCTION__);

	if (card)
		snd_card_free(card);
        platform_set_drvdata(dev, NULL);
	return 0;
}

#define mv88fx_snd_resume	NULL
#define mv88fx_snd_suspend	NULL

static struct platform_driver mv88fx_snd_driver = {
	.probe		= mv88fx_snd_probe,
	.remove		= mv88fx_snd_remove,
	.suspend	= mv88fx_snd_suspend,
	.resume		= mv88fx_snd_resume,
	.driver		= {
		.name	= DRIVER_NAME,
	},

};

static int __init 
mv88fx_snd_init(void)
{
	mv88fx_snd_debug("=>%s", __FUNCTION__);
	return platform_driver_register(&mv88fx_snd_driver);
}

static void __exit mv88fx_snd_exit(void)
{
	mv88fx_snd_debug("=>%s", __FUNCTION__);
	platform_driver_unregister(&mv88fx_snd_driver);	
        
}

MODULE_AUTHOR("Maen Suleiman <maen@marvell.com>");
MODULE_DESCRIPTION("Marvell MV88Fx Alsa Sound driver");
MODULE_LICENSE("GPL");

module_init(mv88fx_snd_init);
module_exit(mv88fx_snd_exit);
