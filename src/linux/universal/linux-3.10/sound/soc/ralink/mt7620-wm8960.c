/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Based on mt7620-sgtl5000.c
 * Copyright 2012 Freescale Semiconductor, Inc.
 * Copyright 2012 Linaro Ltd.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>
#include <linux/pinctrl/consumer.h>

#include "../codecs/wm8960.h"

#define DAI_NAME_SIZE	32

struct mt7620_wm8960_data {
	struct snd_soc_dai_link dai;
	struct snd_soc_card card;
	char codec_dai_name[DAI_NAME_SIZE];
	char platform_name[DAI_NAME_SIZE];
	unsigned int clk_frequency;
};

struct mt7620_priv {
	struct platform_device *pdev;
};
static struct mt7620_priv card_priv;

static const struct snd_soc_dapm_widget mt7620_wm8960_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_SPK("Ext Spk", NULL),
	SND_SOC_DAPM_MIC("AMIC", NULL),
	SND_SOC_DAPM_MIC("DMIC", NULL),
};

static int sample_rate = 44100;
static snd_pcm_format_t sample_format = SNDRV_PCM_FORMAT_S16_LE;

static int mt7620_hifi_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	sample_rate = params_rate(params);
	sample_format = params_format(params);

	return 0;
}

static struct snd_soc_ops mt7620_hifi_ops = {
	.hw_params = mt7620_hifi_hw_params,
};

static int mt7620_wm8960_set_bias_level(struct snd_soc_card *card,
					struct snd_soc_dapm_context *dapm,
					enum snd_soc_bias_level level)
{
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;
	struct mt7620_priv *priv = &card_priv;
	struct mt7620_wm8960_data *data = snd_soc_card_get_drvdata(card);
	struct device *dev = &priv->pdev->dev;
	int ret;

	if (dapm->dev != codec_dai->dev)
		return 0;

	switch (level) {
	case SND_SOC_BIAS_PREPARE:
		if (dapm->bias_level == SND_SOC_BIAS_STANDBY) {
		}
		break;

	case SND_SOC_BIAS_STANDBY:
		if (dapm->bias_level == SND_SOC_BIAS_PREPARE) {
			ret = snd_soc_dai_set_sysclk(codec_dai,
					WM8960_SYSCLK_MCLK, data->clk_frequency,
					SND_SOC_CLOCK_IN);
			if (ret < 0) {
				dev_err(dev,
					"failed to switch away from FLL: %d\n",
					ret);
				return ret;
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

static int mt7620_wm8960_late_probe(struct snd_soc_card *card)
{
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;
	struct mt7620_priv *priv = &card_priv;
	struct mt7620_wm8960_data *data = snd_soc_card_get_drvdata(card);
	struct device *dev = &priv->pdev->dev;
	int ret;

	ret = snd_soc_dai_set_sysclk(codec_dai, WM8960_SYSCLK_MCLK,
			data->clk_frequency, SND_SOC_CLOCK_IN);
	if (ret < 0)
		dev_err(dev, "failed to set sysclk in %s\n", __func__);

	return ret;
}

static int mt7620_wm8960_probe(struct platform_device *pdev)
{
	struct device_node *i2s_np, *codec_np;
	struct platform_device *i2s_pdev;
	struct mt7620_priv *priv = &card_priv;
	struct i2c_client *codec_dev;
	struct mt7620_wm8960_data *data;
	int ret;

	priv->pdev = pdev;

	i2s_np = of_parse_phandle(pdev->dev.of_node, "i2s-controller", 0);
	codec_np = of_parse_phandle(pdev->dev.of_node, "audio-codec", 0);
	if (!i2s_np || !codec_np) {
		dev_err(&pdev->dev, "phandle missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	i2s_pdev = of_find_device_by_node(i2s_np);
	if (!i2s_pdev) {
		dev_err(&pdev->dev, "failed to find SSI platform device\n");
		ret = -EINVAL;
		goto fail;
	}
	codec_dev = of_find_i2c_device_by_node(codec_np);
	if (!codec_dev || !codec_dev->dev.driver) {
		dev_err(&pdev->dev, "failed to find codec platform device\n");
		ret = -EINVAL;
		goto fail;
	}

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto fail;
	}

	data->clk_frequency = 12000000;
	data->dai.name = "HiFi";
	data->dai.stream_name = "HiFi";
	data->dai.codec_dai_name = "wm8960-hifi";
	data->dai.codec_of_node = codec_np;
	data->dai.cpu_dai_name = dev_name(&i2s_pdev->dev);
	data->dai.platform_of_node = i2s_np;
	data->dai.ops = &mt7620_hifi_ops;
	data->dai.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
			    SND_SOC_DAIFMT_CBM_CFM;

	data->card.dev = &pdev->dev;
	ret = snd_soc_of_parse_card_name(&data->card, "model");
	if (ret)
		goto fail;
	ret = snd_soc_of_parse_audio_routing(&data->card, "audio-routing");
	if (ret)
		goto fail;
	data->card.num_links = 1;
	data->card.dai_link = &data->dai;
	data->card.dapm_widgets = mt7620_wm8960_dapm_widgets;
	data->card.num_dapm_widgets = ARRAY_SIZE(mt7620_wm8960_dapm_widgets);

	data->card.late_probe = mt7620_wm8960_late_probe;
	data->card.set_bias_level = mt7620_wm8960_set_bias_level;

	platform_set_drvdata(pdev, &data->card);
	snd_soc_card_set_drvdata(&data->card, data);

	ret = devm_snd_soc_register_card(&pdev->dev, &data->card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n", ret);
		goto fail;
	}

	of_node_put(i2s_np);
	of_node_put(codec_np);

	return 0;
fail:
	if (i2s_np)
		of_node_put(i2s_np);
	if (codec_np)
		of_node_put(codec_np);

	return ret;
}

static int mt7620_wm8960_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id mt7620_wm8960_dt_ids[] = {
	{ .compatible = "mediatek,mt7620-audio-wm8960", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mt7620_wm8960_dt_ids);

static struct platform_driver mt7620_wm8960_driver = {
	.driver = {
		.name = "mt7620-wm8960",
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = mt7620_wm8960_dt_ids,
	},
	.probe = mt7620_wm8960_probe,
	.remove = mt7620_wm8960_remove,
};
module_platform_driver(mt7620_wm8960_driver);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("Freescale i.MX WM8962 ASoC machine driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mt7620-wm8962");
