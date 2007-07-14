/*
 * An hwmon driver for the Analog Devices AD7417/18
 * Copyright 2006 Tower Technologies
 *
 * Author: Alessandro Zummo <a.zummo@towertech.it>
 *
 * Based on lm75.c
 * Copyright 1998-99 Frodo Looijaard <frodol@dds.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/err.h>
#include <linux/mutex.h>

#define DRV_VERSION "0.1"

/* straight from the datasheet */
#define AD741X_TEMP_MIN (-55000)
#define AD741X_TEMP_MAX 125000

/* Addresses to scan */
static unsigned short normal_i2c[] = { 0x28, 0x29, 0x2A, 0x2B, 0x2C,
					0x2D, 0x2E, 0x2F, I2C_CLIENT_END };

/* Insmod parameters */
I2C_CLIENT_INSMOD;

/* AD741X registers */
#define AD741X_REG_TEMP		0x00
#define AD741X_REG_CONF		0x01
#define AD741X_REG_TEMP_HYST	0x02
#define AD741X_REG_TEMP_OS	0x03
#define AD741X_REG_ADC		0x04
#define AD741X_REG_CONF2	0x05

#define AD741X_REG_ADC_CH(x)	(x << 5)

#define AD741X_CH_TEMP		AD741X_REG_ADC_CH(0)
#define AD741X_CH_AIN1		AD741X_REG_ADC_CH(1)
#define AD741X_CH_AIN2		AD741X_REG_ADC_CH(2)
#define AD741X_CH_AIN3		AD741X_REG_ADC_CH(3)
#define AD741X_CH_AIN4		AD741X_REG_ADC_CH(4)

struct ad741x_data {
	struct i2c_client	client;
	struct class_device	*class_dev;
	struct mutex		lock;
	char			valid;		/* !=0 if following fields are valid */
	unsigned long		last_updated;	/* In jiffies */
	u16			temp_input;	/* Register values */
	u16			temp_max;
	u16			temp_hyst;
	u16			in1;
	u16			in2;
	u16			in3;
	u16			in4;
};

static int ad741x_attach_adapter(struct i2c_adapter *adapter);
static int ad741x_detect(struct i2c_adapter *adapter, int address, int kind);
static int ad741x_detach_client(struct i2c_client *client);

static struct i2c_driver ad741x_driver = {
	.driver = {
		.name	= "ad741x",
	},
	.attach_adapter	= ad741x_attach_adapter,
	.detach_client	= ad741x_detach_client,
};

/* TEMP: 0.001C/bit (-55C to +125C)
 * REG: (0.5C/bit, two's complement) << 7
 */
static inline u16 AD741X_TEMP_TO_REG(int temp)
{
	int ntemp = SENSORS_LIMIT(temp, AD741X_TEMP_MIN, AD741X_TEMP_MAX);
	ntemp += (ntemp < 0 ? -250 : 250);
	return (u16)((ntemp / 500) << 7);
}

static inline int AD741X_TEMP_FROM_REG(u16 reg)
{
	/* use integer division instead of equivalent right shift to
	 * guarantee arithmetic shift and preserve the sign
	 */
	return ((s16)reg / 128) * 500;
}

/* All registers are word-sized, except for the configuration registers.
 * AD741X uses a high-byte first convention, which is exactly opposite to
 * the usual practice.
 */
static int ad741x_read(struct i2c_client *client, u8 reg)
{
	if (reg == AD741X_REG_CONF || reg == AD741X_REG_CONF2)
		return i2c_smbus_read_byte_data(client, reg);
	else
		return swab16(i2c_smbus_read_word_data(client, reg));
}

static int ad741x_write(struct i2c_client *client, u8 reg, u16 value)
{
	if (reg == AD741X_REG_CONF || reg == AD741X_REG_CONF2)
		return i2c_smbus_write_byte_data(client, reg, value);
	else
		return i2c_smbus_write_word_data(client, reg, swab16(value));
}

static void ad741x_init_client(struct i2c_client *client)
{
	/* Enable if in shutdown mode */
	int reg = ad741x_read(client, AD741X_REG_CONF);
	if (reg >= 0 && (reg & 0x01))
		ad741x_write(client, AD741X_REG_CONF, reg & 0xfe);
}

static struct ad741x_data *ad741x_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ad741x_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->lock);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
		|| !data->valid) {
		u8 cfg;
		dev_dbg(&client->dev, "starting ad741x update\n");

		data->temp_input = ad741x_read(client, AD741X_REG_TEMP);
		data->temp_max = ad741x_read(client, AD741X_REG_TEMP_OS);
		data->temp_hyst = ad741x_read(client, AD741X_REG_TEMP_HYST);

		/* read config register and clear channel bits */
		cfg = ad741x_read(client, AD741X_REG_CONF);
		cfg &= 0x1F;

		ad741x_write(client, AD741X_REG_CONF, cfg | AD741X_CH_AIN1);
		data->in1 = ad741x_read(client, AD741X_REG_ADC);

		ad741x_write(client, AD741X_REG_CONF, cfg | AD741X_CH_AIN2);
		data->in2 = ad741x_read(client, AD741X_REG_ADC);

		ad741x_write(client, AD741X_REG_CONF, cfg | AD741X_CH_AIN3);
		data->in3 = ad741x_read(client, AD741X_REG_ADC);

		ad741x_write(client, AD741X_REG_CONF, cfg | AD741X_CH_AIN4);
		data->in4 = ad741x_read(client, AD741X_REG_ADC);

		/* restore old configuration value */
		ad741x_write(client, AD741X_REG_CONF, cfg);

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->lock);

	return data;
}

#define show(value) \
static ssize_t show_##value(struct device *dev, struct device_attribute *attr, char *buf)		\
{									\
	struct ad741x_data *data = ad741x_update_device(dev);		\
	return sprintf(buf, "%d\n", AD741X_TEMP_FROM_REG(data->value));	\
}
show(temp_max);
show(temp_hyst);
show(temp_input);

#define show_adc(value)	\
static ssize_t show_##value(struct device *dev, struct device_attribute *attr, char *buf)		\
{								\
	struct ad741x_data *data = ad741x_update_device(dev);	\
	return sprintf(buf, "%d\n", data->value >> 6);		\
}

show_adc(in1);
show_adc(in2);
show_adc(in3);
show_adc(in4);

#define set(value, reg)	\
static ssize_t set_##value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)	\
{								\
	struct i2c_client *client = to_i2c_client(dev);		\
	struct ad741x_data *data = i2c_get_clientdata(client);	\
	int temp = simple_strtoul(buf, NULL, 10);		\
								\
	mutex_lock(&data->lock);				\
	data->value = AD741X_TEMP_TO_REG(temp);			\
	ad741x_write(client, reg, data->value);		\
	mutex_unlock(&data->lock);					\
	return count;						\
}
set(temp_max, AD741X_REG_TEMP_OS);
set(temp_hyst, AD741X_REG_TEMP_HYST);

static DEVICE_ATTR(temp1_max, S_IWUSR | S_IRUGO, show_temp_max, set_temp_max);
static DEVICE_ATTR(temp1_max_hyst, S_IWUSR | S_IRUGO, show_temp_hyst, set_temp_hyst);
static DEVICE_ATTR(temp1_input, S_IRUGO, show_temp_input, NULL);

static DEVICE_ATTR(in1, S_IRUGO, show_in1, NULL);
static DEVICE_ATTR(in2, S_IRUGO, show_in2, NULL);
static DEVICE_ATTR(in3, S_IRUGO, show_in3, NULL);
static DEVICE_ATTR(in4, S_IRUGO, show_in4, NULL);

static int ad741x_attach_adapter(struct i2c_adapter *adapter)
{
	if (!(adapter->class & I2C_CLASS_HWMON))
		return 0;
	return i2c_probe(adapter, &addr_data, ad741x_detect);
}

static int ad741x_detect(struct i2c_adapter *adapter, int address, int kind)
{
	struct i2c_client *client;
	struct ad741x_data *data;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA |
					I2C_FUNC_SMBUS_WORD_DATA))
		goto exit;

	if (!(data = kzalloc(sizeof(struct ad741x_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	client = &data->client;
	client->addr = address;
	client->adapter = adapter;
	client->driver = &ad741x_driver;
	client->flags = 0;

	i2c_set_clientdata(client, data);

	mutex_init(&data->lock);

	/* AD7418 has a curious behaviour on registers 6 and 7. They
	 * both always read 0xC071 and are not documented on the datasheet.
	 * We use them to detect the chip.
	 */
	if (kind < 0) {
		int reg;

		reg = i2c_smbus_read_word_data(client, 0x06);
		if (reg != 0xC071) {
			dev_dbg(&adapter->dev, "failed detection at %d: %x\n", 6, reg);
			err = -ENODEV;
			goto exit_free;
		}

		reg = i2c_smbus_read_word_data(client, 0x07);
		if (reg != 0xC071) {
			dev_dbg(&adapter->dev, "failed detection at %d: %x\n", 7, reg);
			err = -ENODEV;
			goto exit_free;
		}

		reg = i2c_smbus_read_byte_data(client, AD741X_REG_CONF2);

		/* bits 0-5 must be at 0 */
		if (reg & 0x3F) {
			dev_dbg(&adapter->dev, "failed detection at %d: %x\n",
				AD741X_REG_CONF2, reg);
			err = -ENODEV;
		 	goto exit_free;
		}
	}

	strlcpy(client->name, ad741x_driver.driver.name, I2C_NAME_SIZE);

	if ((err = i2c_attach_client(client)))
		goto exit_free;

	dev_info(&client->dev, "chip found, driver version " DRV_VERSION "\n");

	/* Initialize the AD741X chip */
	ad741x_init_client(client);

	/* Register sysfs hooks */
	data->class_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->class_dev)) {
		err = PTR_ERR(data->class_dev);
		goto exit_detach;
	}

	device_create_file(&client->dev, &dev_attr_temp1_max);
	device_create_file(&client->dev, &dev_attr_temp1_max_hyst);
	device_create_file(&client->dev, &dev_attr_temp1_input);
	device_create_file(&client->dev, &dev_attr_in1);
	device_create_file(&client->dev, &dev_attr_in2);
	device_create_file(&client->dev, &dev_attr_in3);
	device_create_file(&client->dev, &dev_attr_in4);

	return 0;

exit_detach:
	i2c_detach_client(client);
exit_free:
	kfree(data);
exit:
	return err;
}

static int ad741x_detach_client(struct i2c_client *client)
{
	struct ad741x_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->class_dev);
	i2c_detach_client(client);
	kfree(data);
	return 0;
}

static int __init ad741x_init(void)
{
	return i2c_add_driver(&ad741x_driver);
}

static void __exit ad741x_exit(void)
{
	i2c_del_driver(&ad741x_driver);
}

MODULE_AUTHOR("Alessandro Zummo <a.zummo@towertech.it>");
MODULE_DESCRIPTION("AD741X driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

module_init(ad741x_init);
module_exit(ad741x_exit);
