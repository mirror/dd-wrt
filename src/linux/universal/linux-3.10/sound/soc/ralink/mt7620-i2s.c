/*
 *  Copyright (C) 2010, Lars-Peter Clausen <lars@metafoo.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/delay.h>

#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/dmaengine_pcm.h>

#include <ralink_regs.h>

#define I2S_REG_CFG0		0x00
#define I2S_REG_CFG0_EN		BIT(31)
#define I2S_REG_CFG0_DMA_EN	BIT(30)
#define I2S_REG_CFG0_BYTE_SWAP	BIT(28)
#define I2S_REG_CFG0_TX_EN	BIT(24)
#define I2S_REG_CFG0_RX_EN	BIT(20)
#define I2S_REG_CFG0_SLAVE	BIT(16)
#define I2S_REG_CFG0_RX_THRES	12
#define I2S_REG_CFG0_TX_THRES	4
#define I2S_REG_CFG0_DFT_THRES	(4 << I2S_REG_CFG0_RX_THRES) | \
					(4 << I2S_REG_CFG0_TX_THRES)

#define I2S_REG_INT_STATUS	0x04
#define I2S_REG_INT_EN		0x08
#define I2S_REG_FF_STATUS	0x0c
#define I2S_REG_WREG		0x10
#define I2S_REG_RREG		0x14
#define I2S_REG_CFG1		0x18

#define I2S_REG_DIVCMP		0x20
#define I2S_REG_DIVINT		0x24
#define I2S_REG_CLK_EN		BIT(31)

struct mt7620_i2s {
	struct resource *mem;
	void __iomem *base;
	dma_addr_t phys_base;

	struct snd_dmaengine_dai_dma_data playback_dma_data;
	struct snd_dmaengine_dai_dma_data capture_dma_data;
};

static inline uint32_t mt7620_i2s_read(const struct mt7620_i2s *i2s,
	unsigned int reg)
{
	return readl(i2s->base + reg);
}

static inline void mt7620_i2s_write(const struct mt7620_i2s *i2s,
	unsigned int reg, uint32_t value)
{
	//printk("i2s --> %p = 0x%08X\n", i2s->base + reg, value);
	writel(value, i2s->base + reg);
}

static int mt7620_i2s_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct mt7620_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

	if (dai->active)
		return 0;

	cfg = mt7620_i2s_read(i2s, I2S_REG_CFG0);
	cfg |= I2S_REG_CFG0_EN;
	mt7620_i2s_write(i2s, I2S_REG_CFG0, cfg);

	return 0;
}

static void mt7620_i2s_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct mt7620_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

	if (dai->active)
		return;

	cfg = mt7620_i2s_read(i2s, I2S_REG_CFG0);
	cfg &= ~I2S_REG_CFG0_EN;
	mt7620_i2s_write(i2s, I2S_REG_CFG0, cfg);
}

static int mt7620_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
	struct snd_soc_dai *dai)
{
	struct mt7620_i2s *i2s = snd_soc_dai_get_drvdata(dai);

	uint32_t cfg;
	uint32_t mask;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		mask = I2S_REG_CFG0_TX_EN;
	else
		mask = I2S_REG_CFG0_RX_EN;

	cfg = mt7620_i2s_read(i2s, I2S_REG_CFG0);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		cfg |= mask;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		cfg &= ~mask;
		break;
	default:
		return -EINVAL;
	}

	if (cfg & (I2S_REG_CFG0_TX_EN | I2S_REG_CFG0_RX_EN))
		cfg |= I2S_REG_CFG0_DMA_EN;
	else
		cfg &= ~I2S_REG_CFG0_DMA_EN;

	mt7620_i2s_write(i2s, I2S_REG_CFG0, cfg);

	return 0;
}

static int mt7620_i2s_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct mt7620_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

	cfg = mt7620_i2s_read(i2s, I2S_REG_CFG0);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		cfg |= I2S_REG_CFG0_SLAVE;
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		cfg &= ~I2S_REG_CFG0_SLAVE;
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_MSB:
		cfg &= ~I2S_REG_CFG0_BYTE_SWAP;
		break;
	case SND_SOC_DAIFMT_LSB:
		cfg |= I2S_REG_CFG0_BYTE_SWAP;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	default:
		return -EINVAL;
	}

	mt7620_i2s_write(i2s, I2S_REG_CFG0, cfg);

	return 0;
}

static int mt7620_i2s_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{

	return 0;
}

unsigned long i2sMaster_inclk_int[11] = {
	78,     56,     52,     39,     28,     26,     19,     14,     13,     9,      6};
unsigned long i2sMaster_inclk_comp[11] = {
	64,     352,    42,     32,     176,    21,     272,    88,     10,     455,    261};


static int mt7620_i2s_set_sysclk(struct snd_soc_dai *dai, int clk_id,
	unsigned int freq, int dir)
{
        struct mt7620_i2s *i2s = snd_soc_dai_get_drvdata(dai);

	printk("Internal REFCLK with fractional division\n");

	mt7620_i2s_write(i2s, I2S_REG_DIVINT, i2sMaster_inclk_int[7]);
	mt7620_i2s_write(i2s, I2S_REG_DIVCMP,
		i2sMaster_inclk_comp[7] | I2S_REG_CLK_EN);

/*	struct mt7620_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	struct clk *parent;
	int ret = 0;

	switch (clk_id) {
	case JZ4740_I2S_CLKSRC_EXT:
		parent = clk_get(NULL, "ext");
		clk_set_parent(i2s->clk_i2s, parent);
		break;
	case JZ4740_I2S_CLKSRC_PLL:
		parent = clk_get(NULL, "pll half");
		clk_set_parent(i2s->clk_i2s, parent);
		ret = clk_set_rate(i2s->clk_i2s, freq);
		break;
	default:
		return -EINVAL;
	}
	clk_put(parent);

	return ret;*/
	return 0;
}

static void mt7620_i2c_init_pcm_config(struct mt7620_i2s *i2s)
{
	struct snd_dmaengine_dai_dma_data *dma_data;

	/* Playback */
	dma_data = &i2s->playback_dma_data;
	dma_data->maxburst = 16;
	dma_data->slave_id = 2; //JZ4740_DMA_TYPE_AIC_TRANSMIT;
	dma_data->addr = i2s->phys_base + I2S_REG_WREG;

	/* Capture */
	dma_data = &i2s->capture_dma_data;
	dma_data->maxburst = 16;
	dma_data->slave_id = 3; //JZ4740_DMA_TYPE_AIC_RECEIVE;
	dma_data->addr = i2s->phys_base + I2S_REG_RREG;
}

static int mt7620_i2s_dai_probe(struct snd_soc_dai *dai)
{
	struct mt7620_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	uint32_t data;

	mt7620_i2c_init_pcm_config(i2s);
	dai->playback_dma_data = &i2s->playback_dma_data;
	dai->capture_dma_data = &i2s->capture_dma_data;

	/* set share pins to i2s/gpio mode and i2c mode */
	data = rt_sysc_r32(0x60);
	data &= 0xFFFFFFE2;
	data |= 0x00000018;
	rt_sysc_w32(data, 0x60);

	printk("Internal REFCLK with fractional division\n");

	mt7620_i2s_write(i2s, I2S_REG_CFG0, I2S_REG_CFG0_DFT_THRES);
	mt7620_i2s_write(i2s, I2S_REG_CFG1, 0);
	mt7620_i2s_write(i2s, I2S_REG_INT_EN, 0);

	mt7620_i2s_write(i2s, I2S_REG_DIVINT, i2sMaster_inclk_int[7]);
	mt7620_i2s_write(i2s, I2S_REG_DIVCMP,
		i2sMaster_inclk_comp[7] | I2S_REG_CLK_EN);

	return 0;
}

static int mt7620_i2s_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static const struct snd_soc_dai_ops mt7620_i2s_dai_ops = {
	.startup = mt7620_i2s_startup,
	.shutdown = mt7620_i2s_shutdown,
	.trigger = mt7620_i2s_trigger,
	.hw_params = mt7620_i2s_hw_params,
	.set_fmt = mt7620_i2s_set_fmt,
	.set_sysclk = mt7620_i2s_set_sysclk,
};

#define JZ4740_I2S_FMTS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			 SNDRV_PCM_FMTBIT_S24_LE)

static struct snd_soc_dai_driver mt7620_i2s_dai = {
	.probe = mt7620_i2s_dai_probe,
	.remove = mt7620_i2s_dai_remove,
	.playback = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_48000,
		.formats = JZ4740_I2S_FMTS,
	},
	.capture = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_48000,
		.formats = JZ4740_I2S_FMTS,
	},
	.symmetric_rates = 1,
	.ops = &mt7620_i2s_dai_ops,
};

static const struct snd_pcm_hardware mt7620_pcm_hardware = {
	.info = SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S8,
	.period_bytes_min	= PAGE_SIZE,
	.period_bytes_max	= 64 * 1024,
	.periods_min		= 2,
	.periods_max		= 128,
	.buffer_bytes_max	= 128 * 1024,
	.fifo_size		= 32,
};

static const struct snd_dmaengine_pcm_config mt7620_dmaengine_pcm_config = {
	.prepare_slave_config = snd_dmaengine_pcm_prepare_slave_config,
	.pcm_hardware = &mt7620_pcm_hardware,
	.prealloc_buffer_size = 256 * PAGE_SIZE,
};

static const struct snd_soc_component_driver mt7620_i2s_component = {
	.name = "mt7620-i2s",
};

static int mt7620_i2s_dev_probe(struct platform_device *pdev)
{
	struct mt7620_i2s *i2s;
	int ret;

	snd_dmaengine_pcm_register(&pdev->dev,
		&mt7620_dmaengine_pcm_config,
		SND_DMAENGINE_PCM_FLAG_COMPAT);

	i2s = kzalloc(sizeof(*i2s), GFP_KERNEL);
	if (!i2s)
		return -ENOMEM;

	i2s->mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!i2s->mem) {
		ret = -ENOENT;
		goto err_free;
	}

	i2s->mem = request_mem_region(i2s->mem->start, resource_size(i2s->mem),
				pdev->name);
	if (!i2s->mem) {
		ret = -EBUSY;
		goto err_free;
	}

	i2s->base = ioremap_nocache(i2s->mem->start, resource_size(i2s->mem));
	if (!i2s->base) {
		ret = -EBUSY;
		goto err_release_mem_region;
	}

	i2s->phys_base = i2s->mem->start;

	platform_set_drvdata(pdev, i2s);
	ret = snd_soc_register_component(&pdev->dev, &mt7620_i2s_component,
					 &mt7620_i2s_dai, 1);

	if (!ret) {
		dev_err(&pdev->dev, "loaded\n");
		return ret;
	}

	dev_err(&pdev->dev, "Failed to register DAI\n");
	iounmap(i2s->base);

err_release_mem_region:
	release_mem_region(i2s->mem->start, resource_size(i2s->mem));
err_free:
	kfree(i2s);

	return ret;
}

static int mt7620_i2s_dev_remove(struct platform_device *pdev)
{
	struct mt7620_i2s *i2s = platform_get_drvdata(pdev);

	snd_soc_unregister_component(&pdev->dev);

	iounmap(i2s->base);
	release_mem_region(i2s->mem->start, resource_size(i2s->mem));

	kfree(i2s);

	snd_dmaengine_pcm_unregister(&pdev->dev);

	return 0;
}

static const struct of_device_id mt7620_i2s_match[] = {
	{ .compatible = "ralink,mt7620a-i2s" },
	{},
};
MODULE_DEVICE_TABLE(of, mt7620_i2s_match);

static struct platform_driver mt7620_i2s_driver = {
	.probe = mt7620_i2s_dev_probe,
	.remove = mt7620_i2s_dev_remove,
	.driver = {
		.name = "mt7620-i2s",
		.owner = THIS_MODULE,
		.of_match_table = mt7620_i2s_match,
	},
};

module_platform_driver(mt7620_i2s_driver);

MODULE_AUTHOR("Lars-Peter Clausen, <lars@metafoo.de>");
MODULE_DESCRIPTION("Ingenic JZ4740 SoC I2S driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mt7620-i2s");
