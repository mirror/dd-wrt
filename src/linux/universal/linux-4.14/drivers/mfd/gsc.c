/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2018 Gateworks Corporation
 *
 * The Gateworks System Controller (GSC) is a family of a multi-function
 * "Power Management and System Companion Device" chips originally designed for
 * use in Gateworks Single Board Computers. The control interface is I2C,
 * at 100kbps, with an interrupt.
 *
 */
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/mfd/gsc.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

/*
 * The GSC suffers from an errata where occasionally during
 * ADC cycles the chip can NAK I2C transactions. To ensure we have reliable
 * register access we place retries around register access.
 */
#define I2C_RETRIES	3

static int gsc_regmap_regwrite(void *context, unsigned int reg,
			       unsigned int val)
{
	struct i2c_client *client = context;
	int retry, ret;

	for (retry = 0; retry < I2C_RETRIES; retry++) {
		ret = i2c_smbus_write_byte_data(client, reg, val);
		/*
		 * -EAGAIN returned when the i2c host controller is busy
		 * -EIO returned when i2c device is busy
		 */
		if (ret != -EAGAIN && ret != -EIO)
			break;
	}

	return 0;
}

static int gsc_regmap_regread(void *context, unsigned int reg,
			      unsigned int *val)
{
	struct i2c_client *client = context;
	int retry, ret;

	for (retry = 0; retry < I2C_RETRIES; retry++) {
		ret = i2c_smbus_read_byte_data(client, reg);
		/*
		 * -EAGAIN returned when the i2c host controller is busy
		 * -EIO returned when i2c device is busy
		 */
		if (ret != -EAGAIN && ret != -EIO)
			break;
	}
	*val = ret & 0xff;

	return 0;
}

static struct regmap_bus regmap_gsc = {
	.reg_write = gsc_regmap_regwrite,
	.reg_read = gsc_regmap_regread,
};

/*
 * gsc_powerdown - API to use GSC to power down board for a specific time
 *
 * secs - number of seconds to remain powered off
 */
static int gsc_powerdown(struct gsc_dev *gsc, unsigned long secs)
{
	int ret;
	unsigned char regs[4];

	dev_info(&gsc->i2c->dev, "GSC powerdown for %ld seconds\n",
		 secs);
	regs[0] = secs & 0xff;
	regs[1] = (secs >> 8) & 0xff;
	regs[2] = (secs >> 16) & 0xff;
	regs[3] = (secs >> 24) & 0xff;
	ret = regmap_bulk_write(gsc->regmap, GSC_TIME_ADD, regs, 4);

	return ret;
}

static ssize_t gsc_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct gsc_dev *gsc = dev_get_drvdata(dev);
	const char *name = attr->attr.name;
	int rz = 0;

	if (strcasecmp(name, "fw_version") == 0)
		rz = sprintf(buf, "%d\n", gsc->fwver);
	else if (strcasecmp(name, "fw_crc") == 0)
		rz = sprintf(buf, "0x%04x\n", gsc->fwcrc);

	return rz;
}

static ssize_t gsc_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct gsc_dev *gsc = dev_get_drvdata(dev);
	const char *name = attr->attr.name;
	int ret;

	if (strcasecmp(name, "powerdown") == 0) {
		long value;

		ret = kstrtol(buf, 0, &value);
		if (ret == 0)
			gsc_powerdown(gsc, value);
	} else
		dev_err(dev, "invalid name '%s\n", name);

	return count;
}

static struct device_attribute attr_fwver =
	__ATTR(fw_version, 0440, gsc_show, NULL);
static struct device_attribute attr_fwcrc =
	__ATTR(fw_crc, 0440, gsc_show, NULL);
static struct device_attribute attr_pwrdown =
	__ATTR(powerdown, 0220, NULL, gsc_store);

static struct attribute *gsc_attrs[] = {
	&attr_fwver.attr,
	&attr_fwcrc.attr,
	&attr_pwrdown.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = gsc_attrs,
};

static const struct of_device_id gsc_of_match[] = {
	{ .compatible = "gw,gsc-v1", .data = (void *)gsc_v1 },
	{ .compatible = "gw,gsc-v2", .data = (void *)gsc_v2 },
	{ .compatible = "gw,gsc-v3", .data = (void *)gsc_v3 },
	{ }
};

static const struct regmap_config gsc_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
	.max_register = 0xf,
};

static const struct regmap_config gsc_regmap_hwmon_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
};

static const struct regmap_irq gsc_irqs[] = {
	REGMAP_IRQ_REG(GSC_IRQ_PB, 0, BIT(GSC_IRQ_PB)),
	REGMAP_IRQ_REG(GSC_IRQ_KEY_ERASED, 0, BIT(GSC_IRQ_KEY_ERASED)),
	REGMAP_IRQ_REG(GSC_IRQ_EEPROM_WP, 0, BIT(GSC_IRQ_EEPROM_WP)),
	REGMAP_IRQ_REG(GSC_IRQ_RESV, 0, BIT(GSC_IRQ_RESV)),
	REGMAP_IRQ_REG(GSC_IRQ_GPIO, 0, BIT(GSC_IRQ_GPIO)),
	REGMAP_IRQ_REG(GSC_IRQ_TAMPER, 0, BIT(GSC_IRQ_TAMPER)),
	REGMAP_IRQ_REG(GSC_IRQ_WDT_TIMEOUT, 0, BIT(GSC_IRQ_WDT_TIMEOUT)),
	REGMAP_IRQ_REG(GSC_IRQ_SWITCH_HOLD, 0, BIT(GSC_IRQ_SWITCH_HOLD)),
};

static const struct regmap_irq_chip gsc_irq_chip = {
	.name = "gateworks-gsc",
	.irqs = gsc_irqs,
	.num_irqs = ARRAY_SIZE(gsc_irqs),
	.num_regs = 1,
	.status_base = GSC_IRQ_STATUS,
	.mask_base = GSC_IRQ_ENABLE,
	.mask_invert = true,
	.ack_base = GSC_IRQ_STATUS,
	.ack_invert = true,
};

static int gsc_of_probe(struct device_node *np, struct gsc_dev *gsc)
{
	const struct of_device_id *of_id;

	if (!np)
		return -ENODEV;

	of_id = of_match_device(gsc_of_match, gsc->dev);
	if (of_id)
		gsc->type = (enum gsc_type)of_id->data;

	return 0;
}

static int
gsc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct gsc_dev *gsc;
	int ret;
	unsigned int reg;

	gsc = devm_kzalloc(dev, sizeof(*gsc), GFP_KERNEL);
	if (!gsc)
		return -ENOMEM;

	gsc->dev = &client->dev;
	gsc->i2c = client;
	gsc->irq = client->irq;
	i2c_set_clientdata(client, gsc);

	ret = gsc_of_probe(dev->of_node, gsc);
	if (ret < 0)
		return ret;

	gsc->regmap = devm_regmap_init(dev, &regmap_gsc, client,
				       &gsc_regmap_config);
	if (IS_ERR(gsc->regmap))
		return PTR_ERR(gsc->regmap);

	if (regmap_read(gsc->regmap, GSC_FW_VER, &reg))
		return -EIO;
	gsc->fwver = reg;

	regmap_read(gsc->regmap, GSC_FW_CRC, &reg);
	gsc->fwcrc = reg;
	regmap_read(gsc->regmap, GSC_FW_CRC + 1, &reg);
	gsc->fwcrc |= reg << 8;

	gsc->i2c_hwmon = i2c_new_dummy(client->adapter, GSC_HWMON);
	if (!gsc->i2c_hwmon) {
		dev_err(dev, "Failed to allocate I2C device for HWMON\n");
		return -ENODEV;
	}
	i2c_set_clientdata(gsc->i2c_hwmon, gsc);

	gsc->regmap_hwmon = devm_regmap_init(dev, &regmap_gsc, gsc->i2c_hwmon,
					     &gsc_regmap_hwmon_config);
	if (IS_ERR(gsc->regmap_hwmon)) {
		ret = PTR_ERR(gsc->regmap_hwmon);
		dev_err(dev, "failed to allocate register map: %d\n", ret);
		goto err_regmap;
	}

	ret = devm_regmap_add_irq_chip(dev, gsc->regmap, gsc->irq,
				       IRQF_ONESHOT | IRQF_SHARED |
				       IRQF_TRIGGER_FALLING, 0,
				       &gsc_irq_chip, &gsc->irq_chip_data);
	if (ret)
		goto err_regmap;

	dev_info(dev, "Gateworks System Controller v%d: fw 0x%04x\n",
		 gsc->fwver, gsc->fwcrc);

	ret = sysfs_create_group(&dev->kobj, &attr_group);
	if (ret)
		dev_err(dev, "failed to create sysfs attrs\n");

	ret = of_platform_populate(dev->of_node, NULL, NULL, dev);
	if (ret)
		goto err_sysfs;

	return 0;

err_sysfs:
	sysfs_remove_group(&dev->kobj, &attr_group);
err_regmap:
	i2c_unregister_device(gsc->i2c_hwmon);

	return ret;
}

static int gsc_remove(struct i2c_client *client)
{
	struct gsc_dev *gsc = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &attr_group);
	i2c_unregister_device(gsc->i2c_hwmon);

	return 0;
}

static struct i2c_driver gsc_driver = {
	.driver = {
		.name	= "gateworks-gsc",
		.of_match_table = of_match_ptr(gsc_of_match),
	},
	.probe		= gsc_probe,
	.remove		= gsc_remove,
};

module_i2c_driver(gsc_driver);

MODULE_AUTHOR("Tim Harvey <tharvey@gateworks.com>");
MODULE_DESCRIPTION("I2C Core interface for GSC");
MODULE_LICENSE("GPL v2");
