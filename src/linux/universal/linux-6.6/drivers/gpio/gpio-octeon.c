/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2011, 2012 Cavium Inc.
 */

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/gpio/driver.h>
#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include "gpiolib.h"

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-gpio-defs.h>

#define RX_DAT 0x80
#define TX_SET 0x88
#define TX_CLEAR 0x90

struct octeon_gpio {
	struct gpio_chip chip;
	u64 register_base;
	unsigned int (*cfg_reg)(unsigned int);
};

/*
 * The address offset of the GPIO configuration register for a given
 * line.
 */
static unsigned int bit_cfg_reg38(unsigned int offset)
{
	/*
	 * The register stride is 8, with a discontinuity after the
	 * first 16.
	 */
	if (offset < 16)
		return 8 * offset;
	else
		return 8 * (offset - 16) + 0x100;
}

static unsigned int bit_cfg_reg78(unsigned int gpio)
{
	return (8 * gpio) + 0x100;
}

static int octeon_gpio_dir_in(struct gpio_chip *chip, unsigned offset)
{
	struct octeon_gpio *gpio = gpiochip_get_data(chip);

	cvmx_write_csr(gpio->register_base + gpio->cfg_reg(offset), 0);
	return 0;
}

static void octeon_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct octeon_gpio *gpio = gpiochip_get_data(chip);
	u64 mask = 1ull << offset;
	u64 reg = gpio->register_base + (value ? TX_SET : TX_CLEAR);
	cvmx_write_csr(reg, mask);
}

static int octeon_gpio_dir_out(struct gpio_chip *chip, unsigned offset,
			       int value)
{
	struct octeon_gpio *gpio = gpiochip_get_data(chip);
	union cvmx_gpio_bit_cfgx cfgx;

	octeon_gpio_set(chip, offset, value);

	cfgx.u64 = 0;
	cfgx.s.tx_oe = 1;

	cvmx_write_csr(gpio->register_base + gpio->cfg_reg(offset), cfgx.u64);
	return 0;
}

static int octeon_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct octeon_gpio *gpio = gpiochip_get_data(chip);
	u64 read_bits = cvmx_read_csr(gpio->register_base + RX_DAT);

	return ((1ull << offset) & read_bits) != 0;
}

static int octeon_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct of_phandle_args oirq;

	if (offset >= 16)
		return -ENXIO;

	oirq.np = chip->parent->of_node;
	oirq.args_count = 2;
	oirq.args[0] = offset;
	oirq.args[1] = 8; /* Level Low*/

	return irq_create_of_mapping(&oirq);
}

struct match_data {
	unsigned int (*bit_cfg)(unsigned int gpio);
	int (*irq_init_gpio)(struct device_node *n, struct device_node *p);
};

int octeon_irq_init_gpio(struct device_node *gpio_node, struct device_node *parent);
int octeon_irq_init_gpio78(struct device_node *gpio_node, struct device_node *parent);

static const struct match_data md38 = {
	.bit_cfg = bit_cfg_reg38,
	.irq_init_gpio = octeon_irq_init_gpio
};
static const struct match_data md78 = {
	.bit_cfg = bit_cfg_reg78,
	.irq_init_gpio = octeon_irq_init_gpio78
};

static struct of_device_id octeon_gpio_match[] = {
	{
		.compatible = "cavium,octeon-3860-gpio",
		.data = &md38,
	},
	{
		.compatible = "cavium,octeon-7890-gpio",
		.data = &md78,
	},
	{},
};
MODULE_DEVICE_TABLE(of, octeon_gpio_match);

static int octeon_gpio_probe(struct platform_device *pdev)
{
	struct octeon_gpio *gpio;
	struct gpio_chip *chip;
	void __iomem *reg_base;
	const struct of_device_id *of_id;
	struct device_node *irq_parent;
	const struct match_data *md;
	int err = 0;

	of_id = of_match_device(octeon_gpio_match, &pdev->dev);
	if (!of_id)
		return -EINVAL;
	md = of_id->data;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;
	chip = &gpio->chip;
	gpio->cfg_reg = md->bit_cfg;

	reg_base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg_base))
		return PTR_ERR(reg_base);

	gpio->register_base = (u64)reg_base;
	pdev->dev.platform_data = chip;

	irq_parent = of_irq_find_parent(pdev->dev.of_node);
	if (irq_parent) {
		err = md->irq_init_gpio(pdev->dev.of_node, irq_parent);
		if (err)
			dev_err(&pdev->dev, "Error: irq init failed %d\n", err);
	}

	chip->label = "octeon-gpio";
	chip->parent = &pdev->dev;
	chip->fwnode = of_node_to_fwnode(pdev->dev.of_node);
	chip->owner = THIS_MODULE;
	chip->base = of_node_to_nid(pdev->dev.of_node) ? -1 : 0;
	chip->can_sleep = false;
	chip->ngpio = 32;
	chip->direction_input = octeon_gpio_dir_in;
	chip->get = octeon_gpio_get;
	chip->direction_output = octeon_gpio_dir_out;
	chip->set = octeon_gpio_set;
	chip->of_gpio_n_cells = 2;
	chip->of_xlate = of_gpio_simple_xlate;
	chip->to_irq = octeon_gpio_to_irq;
	err = devm_gpiochip_add_data(&pdev->dev, chip, gpio);
	if (err)
		return err;

	dev_info(&pdev->dev, "OCTEON GPIO: base = %d\n", chip->base);
	return 0;
}


static struct platform_driver octeon_gpio_driver = {
	.driver = {
		.name		= "octeon_gpio",
		.of_match_table = octeon_gpio_match,
	},
	.probe		= octeon_gpio_probe,
};

module_platform_driver(octeon_gpio_driver);

MODULE_DESCRIPTION("Cavium Inc. OCTEON GPIO Driver");
MODULE_AUTHOR("David Daney");
MODULE_LICENSE("GPL");
