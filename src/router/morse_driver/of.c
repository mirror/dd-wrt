/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/of_gpio.h>

#include "of.h"

int morse_of_probe(struct device *dev, struct morse_hw_cfg *cfg,
		   const struct of_device_id *match_table)
{
	struct device_node *np = dev->of_node;
	const struct of_device_id *of_id;
	int gpio_pin;

	if (np) {
		of_id = of_match_node(match_table, np);
		if (of_id) {
			dev_info(dev, "%s: Reading gpio pins configuration from device tree\n",
				 __func__);
			gpio_pin = of_get_named_gpio(np, "power-gpios", 0);
			cfg->mm_wake_gpio = gpio_pin;

			gpio_pin = of_get_named_gpio(np, "power-gpios", 1);
			cfg->mm_ps_async_gpio = gpio_pin;

			cfg->mm_ps_gpios_supported = !!(cfg->mm_wake_gpio > 0 &&
							cfg->mm_ps_async_gpio > 0);

			/* Don't error on no PS config - this might be valid, e.g. AP only device */
			if (!cfg->mm_ps_gpios_supported)
				dev_err(dev, "%s: optional property power-gpios incomplete, powersave won't be supported\n",
					__func__);

			gpio_pin = of_get_named_gpio(np, "reset-gpios", 0);
			if (gpio_pin < 0) {
				dev_err(dev, "%s: Required property reset-gpios not found in device tree\n",
					__func__);
				return -ENOENT;
			}
			cfg->mm_reset_gpio = gpio_pin;

			/* Don't error on spi-irq-gpios - this might not be a SPI device */
			gpio_pin = of_get_named_gpio(np, "spi-irq-gpios", 0);
			cfg->mm_spi_irq_gpio = gpio_pin;
			return 0;
		}
		dev_err(dev, "%s: Couldn't match device table\n", __func__);
	} else {
		dev_err(dev, "%s: Device node not found\n", __func__);
	}
	dev_info(dev, "%s: No pin configs found\n", __func__);
	return -ENOENT;
}
