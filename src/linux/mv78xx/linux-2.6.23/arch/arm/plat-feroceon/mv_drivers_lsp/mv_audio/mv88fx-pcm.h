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
#define DRIVER_NAME	"mv88fx_snd"

#define MV88FX_SND_DEBUG
#ifdef MV88FX_SND_DEBUG
#define mv88fx_snd_debug(fmt, arg...) printk(KERN_DEBUG fmt, ##arg)
#else
	#define mv88fx_snd_debug(a...)
#endif


typedef struct {
	unsigned int base;
	unsigned int size;
} _audio_mem_info;

struct mv88fx_snd_stream {
	struct snd_pcm_substream	*substream;
        struct device*		dev;
	int 	direction; 	/* playback or capture */ 
	#define PLAYBACK	0
	#define CAPTURE		1
	unsigned int	dig_mode;	/* i2s,spdif,both*/ 
	#define I2S		1
	#define SPDIF		2
        int	stereo;		/* mono , stereo*/
        int	mono_mode;	/* both mono, left mono, right mono*/
	#define MONO_BOTH	0
	#define MONO_LEFT	1
	#define MONO_RIGHT	2
	int	clock_src;
	#define DCO_CLOCK	0
	#define SPCR_CLOCK	1
	#define EXTERN_CLOCK	2
	int	rate;		
	int	stat_mem;	/* Channel status source*/
	int	format;
        #define SAMPLE_32IN32	0
	#define SAMPLE_24IN32	1
	#define SAMPLE_20IN32	2
	#define SAMPLE_16IN32	3
	#define SAMPLE_16IN16	4
	unsigned int	dma_addr;
	unsigned int	dma_size;
	unsigned int	period_size;
	unsigned int spdif_status[4];	/* SPDIF status */
	unsigned char 	*area;	/* virtual pointer */
	dma_addr_t 	addr;	/* physical address */

	
};

struct mv88fx_snd_chip {
	struct snd_card *card;
	struct device	*dev;
	struct mv88fx_snd_platform_data	*pdata;	/* platform dara*/
	struct mv88fx_snd_stream	*stream[2];	/* run time values*/
	struct mv88fx_snd_stream	*stream_defaults[2]; /* default values*/
        spinlock_t	reg_lock;	/* Register access spinlock */
	struct resource	*res;		/* resource for IRQ and base*/
	void __iomem	*base;		/* base address of the host */
	int	irq;
	int	loopback;		/* When Loopback is enabled, playback 
					  data is looped back to be recorded */
	int	ch_stat_valid;		/* Playback SPDIF channel validity bit 
					value when REG selected */
        int	burst;			/* DMA Burst Size */

	#define SPDIF_MEM_STAT		0
	#define SPDIF_REG_STAT		1
	unsigned int dco_ctrl_offst;
	int	pcm_mode;	/* pcm, nonpcm*/
	#define PCM		0
	#define NON_PCM		1
	_audio_mem_info	*mem_array;
	
};

#define MV88FX_SND_MIN_PERIODS		8
#define MV88FX_SND_MAX_PERIODS		16
#define	MV88FX_SND_MIN_PERIOD_BYTES	0x4000
#define	MV88FX_SND_MAX_PERIOD_BYTES	0x4000


/* read/write registers */

#define mv88fx_snd_writel(chip, offs, val)	\
		writel((val), (0xf1000000 + offs))

static inline unsigned int mv88fx_snd_readl(struct mv88fx_snd_chip *chip, 
					   unsigned int offs)
{
	unsigned int val = readl((0xf1000000 + offs));
	return (val);
}
#define mv88fx_snd_bitset(chip, offs, bitmask)	\
		writel( (readl(0xf1000000 + offs) | (bitmask)),	\
			0xf1000000 + offs)

#define mv88fx_snd_bitreset(chip, offs, bitmask)	\
		writel( (readl(0xf1000000 + offs) & (~(bitmask))), \
			0xf1000000 + offs)


