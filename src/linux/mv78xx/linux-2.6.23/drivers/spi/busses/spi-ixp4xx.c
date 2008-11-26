/*
 * drivers/spi/spi-ixp4xx.c
 *
 * Intel's IXP4xx XScale NPU chipsets (IXP420, 421, 422, 425) do not have
 * an on board SPI controller but provide 16 GPIO pins that are often
 * used to create an SPI bus. This driver provides an spi_adapter 
 * interface that plugs in under algo_bit and drives the GPIO pins
 * as instructed by the alogorithm driver.
 *
 * Author: Deepak Saxena <dsaxena@plexity.net>
 *
 * Copyright (c) 2003-2004 MontaVista Software Inc.
 *
 * This file is licensed under the terms of the GNU General Public 
 * License version 2. This program is licensed "as is" without any 
 * warranty of any kind, whether express or implied.
 *
 * NOTE: Since different platforms will use different GPIO pins for
 *       SPI, this driver uses an IXP4xx-specific platform_data
 *       pointer to pass the GPIO numbers to the driver. This 
 *       allows us to support all the different IXP4xx platforms
 *       w/o having to put #ifdefs in this driver.
 *
 *       See arch/arm/mach-ixp4xx/ixdp425.c for an example of building a 
 *       device list and filling in the ixp4xx_spi_pins data structure 
 *       that is passed as the platform_data to this driver.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-algo-bit.h>

#include <asm/hardware.h>	/* Pick up IXP4xx-specific bits */

struct ixp4xx_spi_data {
	struct spi_adapter adapter;
	struct spi_algo_bit_data algo_data;
};


static void ixp4xx_bit_setspis(void *data, int index)
{
    gpio_line_set( ((struct ixp4xx_spi_pins*)data)->spis_pin, !(0 == index) );
}

static void ixp4xx_bit_setspic(void *data, int val)
{
    gpio_line_set( ((struct ixp4xx_spi_pins*)data)->spic_pin, val );
}

static void ixp4xx_bit_setspid(void *data, int val)
{
	gpio_line_set( ((struct ixp4xx_spi_pins*)data)->spid_pin, val );
}

static int ixp4xx_bit_getspiq(void *data)
{
    int val;
    gpio_line_get( ((struct ixp4xx_spi_pins*)data)->spiq_pin, &val );
    return val;
}	

static int ixp4xx_spi_probe(struct device *dev)
{
	int err;
	struct platform_device *plat_dev = to_platform_device(dev);
	struct ixp4xx_spi_pins *gpio = plat_dev->dev.platform_data;
	struct ixp4xx_spi_data *drv_data = 
		kmalloc(sizeof(struct ixp4xx_spi_data), GFP_KERNEL);

	if(!drv_data)
		return -ENOMEM;

	memzero(drv_data, sizeof(struct ixp4xx_spi_data));

	/*
	 * We could make a lot of these structures static, but
	 * certain platforms may have multiple GPIO-based SPI
	 * buses for various device domains, so we need per-device
	 * algo_data->data. 
	 */
	drv_data->algo_data.data   = gpio;
	drv_data->algo_data.setspis= ixp4xx_bit_setspis;
	drv_data->algo_data.setspic= ixp4xx_bit_setspic;
	drv_data->algo_data.setspid= ixp4xx_bit_setspid;
	drv_data->algo_data.getspiq= ixp4xx_bit_getspiq;
	drv_data->algo_data.udelay = 10;
	drv_data->algo_data.mdelay = 10;
	drv_data->algo_data.timeout = 10;

	drv_data->adapter.id = SPI_HW_B_IXP4XX;
	drv_data->adapter.algo_data = &drv_data->algo_data;

	drv_data->adapter.dev.parent = &plat_dev->dev;

	gpio_line_config(gpio->spis_pin, IXP4XX_GPIO_OUT);
	gpio_line_config(gpio->spic_pin, IXP4XX_GPIO_OUT);
	gpio_line_config(gpio->spid_pin, IXP4XX_GPIO_OUT);
	gpio_line_config(gpio->spiq_pin, IXP4XX_GPIO_IN);
	gpio_line_set(gpio->spis_pin, 1);
	gpio_line_set(gpio->spic_pin, 1);
	gpio_line_set(gpio->spid_pin, 0);

	if ((err = spi_bit_add_bus(&drv_data->adapter) != 0)) {
		printk(KERN_ERR "ERROR: Could not install %s\n", dev->bus_id);

		kfree(drv_data);
		return err;
	}

	dev_set_drvdata(&plat_dev->dev, drv_data);

	return 0;
}

static int ixp4xx_spi_remove(struct device *dev)
{
	struct platform_device *plat_dev = to_platform_device(dev);
	struct ixp4xx_spi_data *drv_data = dev_get_drvdata(&plat_dev->dev);

	dev_set_drvdata(&plat_dev->dev, NULL);

	spi_bit_del_bus(&drv_data->adapter);

	kfree(drv_data);

	return 0;
}

static struct device_driver ixp4xx_spi_driver = {
	.name		= "IXP4XX-SPI",
	.bus		= &platform_bus_type,
	.probe		= ixp4xx_spi_probe,
	.remove		= ixp4xx_spi_remove,
};

static int __init ixp4xx_spi_init(void)
{
    return driver_register(&ixp4xx_spi_driver);
}

static void __exit ixp4xx_spi_exit(void)
{
    driver_unregister(&ixp4xx_spi_driver);
}

module_init(ixp4xx_spi_init);
module_exit(ixp4xx_spi_exit);

MODULE_DESCRIPTION("GPIO-based SPI adapter for IXP4xx systems");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Barnabas Kalman <ba...@sednet.hu>");

