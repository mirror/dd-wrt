/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2011 Cavium Inc. 
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/gpio.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-gpio-defs.h>

#define DRV_VERSION "1.0"
#define DRV_DESCRIPTION "Cavium Inc. OCTEON GPIO Driver"

#define RX_DAT 0x80
#define TX_SET 0x88
#define TX_CLEAR 0x90
/*
 * The address offset of the GPIO configuration register for a given
 * line.
 */
static unsigned int bit_cfg_reg(unsigned int gpio)
{
	if (gpio < 16)
		return 8 * gpio;
	else
		return 8 * (gpio - 16) + 0x100;
}

struct octeon_gpio {
	struct gpio_chip chip;
	u64 register_base;
};

static int octeon_gpio_dir_in(struct gpio_chip *chip, unsigned offset)
{
	struct octeon_gpio *gpio = container_of(chip, struct octeon_gpio, chip);

	cvmx_write_csr(gpio->register_base + bit_cfg_reg(offset), 0);
	return 0;
}

static void octeon_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct octeon_gpio *gpio = container_of(chip, struct octeon_gpio, chip);
	u64 mask = 1ull << offset;
	u64 reg = gpio->register_base + (value ? TX_SET : TX_CLEAR);
	cvmx_write_csr(reg, mask);
}

static int octeon_gpio_dir_out(struct gpio_chip *chip, unsigned offset,
			       int value)
{
	struct octeon_gpio *gpio = container_of(chip, struct octeon_gpio, chip);
	union cvmx_gpio_bit_cfgx cfgx;


	octeon_gpio_set(chip, offset, value);

	cfgx.u64 = 0;
	cfgx.s.tx_oe = 1;

	cvmx_write_csr(gpio->register_base + bit_cfg_reg(offset), cfgx.u64);
	return 0;
}

static int octeon_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct octeon_gpio *gpio = container_of(chip, struct octeon_gpio, chip);
	u64 read_bits = cvmx_read_csr(gpio->register_base + RX_DAT);

	return ((1ull << offset) & read_bits) != 0;
}

static int __init octeon_gpio_probe(struct platform_device *pdev)
{
	struct octeon_gpio *gpio;
	struct gpio_chip *chip;
	struct resource *res_mem;
	int err = 0;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;
	chip = &gpio->chip;

	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res_mem == NULL) {
		dev_err(&pdev->dev, "found no memory resource\n");
		err = -ENXIO;
		goto out;
	}
	if (!devm_request_mem_region(&pdev->dev, res_mem->start,
					resource_size(res_mem),
				     res_mem->name)) {
		dev_err(&pdev->dev, "request_mem_region failed\n");
		err = -ENXIO;
		goto out;
	}
	gpio->register_base = (u64)ioremap(res_mem->start,
					   resource_size(res_mem));


	pdev->dev.platform_data = chip;
	chip->label = "octeon-gpio";
	chip->dev = &pdev->dev;
	chip->owner = THIS_MODULE;
	chip->base = 0;
	chip->can_sleep = 0;

	if (OCTEON_IS_MODEL(OCTEON_CN66XX))
		chip->ngpio = 18;
	else
		chip->ngpio = 16;

	chip->direction_input = octeon_gpio_dir_in;
	chip->get = octeon_gpio_get;
	chip->direction_output = octeon_gpio_dir_out;
	chip->set = octeon_gpio_set;
	err = gpiochip_add(chip);
	if (err)
		goto out;

	dev_info(&pdev->dev, "probed\n");
out:
	return err;
}

static int __exit octeon_gpio_remove(struct platform_device *pdev)
{
	struct gpio_chip *chip = pdev->dev.platform_data;
	return gpiochip_remove(chip);
}

static struct of_device_id octeon_gpio_match[] = {
	{
		.compatible = "cavium,octeon-3860-gpio",
	},
	{},
};
MODULE_DEVICE_TABLE(of, octeon_mgmt_match);

static struct platform_driver octeon_gpio_driver = {
	.driver = {
		.name		= "octeon_gpio",
		.owner		= THIS_MODULE,
		.of_match_table = octeon_gpio_match,
	},
	.probe		= octeon_gpio_probe,
	.remove		= __exit_p(octeon_gpio_remove),
};

static int __init octeon_gpio_mod_init(void)
{
	return platform_driver_register(&octeon_gpio_driver);
}
module_init(octeon_gpio_mod_init);

static void __exit octeon_gpio_mod_exit(void)
{
	platform_driver_unregister(&octeon_gpio_driver);
}
module_exit(octeon_gpio_mod_exit);

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("David Daney");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
