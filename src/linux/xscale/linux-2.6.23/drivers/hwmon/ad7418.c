/*
    ad7418.c - Part of lm_sensors, Linux kernel modules for hardware
             monitoring
    Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/err.h>
#include <linux/delay.h>
#include "ad7418.h"


/* Addresses to scan */
static unsigned short normal_i2c[] = { 0x28, I2C_CLIENT_END };

/* Insmod parameters */
I2C_CLIENT_INSMOD_1(ad7418);

/* Many AD7418 constants specified below */

/* The AD7418 registers */
#define AD7418_REG_TEMP		0x00
#define AD7418_REG_CONF		0x01
#define AD7418_REG_TEMP_HYST	0x02
#define AD7418_REG_TEMP_OS	0x03
#define AD7418_REG_VOLT 0x04
#define AD7418_REG_CONF2 0x05
#define AD7418_CHANNEL_TEMP 0x00
#define AD7418_CHANNEL_VOLT  0x04

/* Each client has this additional data */
struct ad7418_data {
	struct i2c_client	client;
	struct class_device *class_dev;
	struct semaphore	update_lock;
	char			valid;		/* !=0 if following fields are valid */
	unsigned long		last_updated;	/* In jiffies */
	u16			temp_input;	/* Register values */
	u16			temp_max;
	u16			temp_hyst;
	u16			volt;
	u8 			channel;
	unsigned long channel_last_updated;
};

static int ad7418_attach_adapter(struct i2c_adapter *adapter);
static int ad7418_detect(struct i2c_adapter *adapter, int address, int kind);
static void ad7418_init_client(struct i2c_client *client);
static int ad7418_detach_client(struct i2c_client *client);
static int ad7418_read_value(struct i2c_client *client, u8 reg);
static int ad7418_write_value(struct i2c_client *client, u8 reg, u16 value);
static struct ad7418_data *ad7418_update_device(struct device *dev);
static void ad7418_set_channel(struct i2c_client *client, int channel);

/* This is the driver that will be inserted */
static struct i2c_driver ad7418_driver = {
	.driver = {
	.owner		= THIS_MODULE,
	.name		= "ad7418",
	},
	.id		= I2C_DRIVERID_AD7418,
//	.flags		= I2C_DF_NOTIFY,
	.attach_adapter	= ad7418_attach_adapter,
	.detach_client	= ad7418_detach_client,
};

#define show(value)	\
static ssize_t show_##value(struct device *dev, struct device_attribute *attr, char *buf)		\
{									\
	ad7418_set_channel(to_i2c_client(dev), AD7418_CHANNEL_TEMP); \
	struct ad7418_data *data = ad7418_update_device(dev);		\
	return sprintf(buf, "%d\n", AD7418_TEMP_FROM_REG(data->value));	\
}
show(temp_max);
show(temp_hyst);
show(temp_input);

#define set(value, reg)	\
static ssize_t set_##value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)	\
{								\
	struct i2c_client *client = to_i2c_client(dev);		\
	struct ad7418_data *data = i2c_get_clientdata(client);	\
	int temp = simple_strtoul(buf, NULL, 10);		\
								\
	down(&data->update_lock);				\
	data->value = AD7418_TEMP_TO_REG(temp);			\
	ad7418_write_value(client, reg, data->value);		\
	up(&data->update_lock);					\
	return count;						\
}
set(temp_max, AD7418_REG_TEMP_OS);
set(temp_hyst, AD7418_REG_TEMP_HYST);

#define adc(value) \
static ssize_t adc_##value(struct device *dev, struct device_attribute *attr, char *buf)   \
{                 \
	ad7418_set_channel(to_i2c_client(dev), AD7418_CHANNEL_VOLT); \
  struct ad7418_data *data = ad7418_update_device(dev);   \
  return sprintf(buf, "%d\n", ADC_TO_GW2342VOLT(data->value)); \
}
adc(volt);

static DEVICE_ATTR(temp_max, S_IWUSR | S_IRUGO, show_temp_max, set_temp_max);
static DEVICE_ATTR(temp_max_hyst, S_IWUSR | S_IRUGO, show_temp_hyst, set_temp_hyst);
static DEVICE_ATTR(temp_input, S_IRUGO, show_temp_input, NULL);
static DEVICE_ATTR(volt, S_IRUGO, adc_volt, NULL);

static void ad7418_set_channel(struct i2c_client *client, int channel) {
  struct ad7418_data *data = i2c_get_clientdata(client);

  if (data->channel != channel) {
    down(&data->update_lock);
    u8 conf = ad7418_read_value(client, AD7418_REG_CONF);
    conf &= 0x1f;
    conf |= (channel << 5);

    ad7418_write_value(client, AD7418_REG_CONF, conf);
    data->channel = channel;
    data->channel_last_updated = jiffies;
    data->valid = 0;

    udelay(1000);

    up(&data->update_lock);
  }
}

static int ad7418_attach_adapter(struct i2c_adapter *adapter)
{
/*	if (!(adapter->class & I2C_CLASS_HWMON))
		return 0;
*/
	return i2c_probe(adapter, &addr_data, ad7418_detect);
}

/* This function is called by i2c_probe */
static int ad7418_detect(struct i2c_adapter *adapter, int address, int kind)
{
	int i;
	struct i2c_client *new_client;
	struct ad7418_data *data;
	int err = 0;
	const char *name = "";

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA |
				     I2C_FUNC_SMBUS_WORD_DATA))
		{
		printk(KERN_EMERG "check functionality failed\n");
		goto exit;
		}
	/* OK. For now, we presume we have a valid client. We now create the
	   client structure, even though we cannot fill it completely yet.
	   But it allows us to access ad7418_{read,write}_value. */
	if (!(data = kzalloc(sizeof(struct ad7418_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	new_client = &data->client;
	i2c_set_clientdata(new_client, data);
	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &ad7418_driver;
	new_client->flags = 0;

	/* Now, we do the remaining detection. There is no identification-
	   dedicated register so we have to rely on several tricks:
	   unused bits, registers cycling over 8-address boundaries,
	   addresses 0x04-0x07 returning the last read value.
	   The cycling+unused addresses combination is not tested,
	   since it would significantly slow the detection down and would
	   hardly add any value. */

	if (kind < 0) {
		int cur, conf, hyst, os;

		/* Unused addresses */
		cur = i2c_smbus_read_word_data(new_client, 0);
		conf = i2c_smbus_read_byte_data(new_client, 1);
		hyst = i2c_smbus_read_word_data(new_client, 2);
		os = i2c_smbus_read_word_data(new_client, 3);

		/* Addresses cycling */
		for (i = 0; i < 0x1f; i++)
			if (
          (i2c_smbus_read_byte_data
           (new_client, i * 8 + 1) != conf)
          ||
          (i2c_smbus_read_word_data
           (new_client, i * 8 + 2) != hyst)
          ||
          (i2c_smbus_read_word_data
           (new_client, i * 8 + 3) != os))
        goto exit_free;
	}

	/* Determine the chip type - only one kind supported! */
	if (kind <= 0)
		kind = ad7418;

	if (kind == ad7418) {
		name = "ad7418";
	}

	/* Fill in the remaining client fields and put it into the global list */
	strlcpy(new_client->name, name, I2C_NAME_SIZE);
	data->valid = 0;
	init_MUTEX(&data->update_lock);

	/* Tell the I2C layer a new client has arrived */
	if ((err = i2c_attach_client(new_client)))
		goto exit_free;

	/* Initialize the AD7418 chip */
	ad7418_init_client(new_client);
	
	/* Register sysfs hooks */
	data->class_dev = hwmon_device_register(&new_client->dev);
	if (IS_ERR(data->class_dev)) {
		err = PTR_ERR(data->class_dev);
		goto exit_detach;
	}

	device_create_file(&new_client->dev, &dev_attr_temp_max);
	device_create_file(&new_client->dev, &dev_attr_temp_max_hyst);
	device_create_file(&new_client->dev, &dev_attr_temp_input);
	device_create_file(&new_client->dev, &dev_attr_volt);

	return 0;

exit_detach:
	i2c_detach_client(new_client);
exit_free:
	kfree(data);
exit:
	return err;
}

static int ad7418_detach_client(struct i2c_client *client)
{
	struct ad7418_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->class_dev);
	i2c_detach_client(client);
	kfree(data);
	return 0;
}

/* All registers are word-sized, except for the configuration register.
   AD7418 uses a high-byte first convention, which is exactly opposite to
   the usual practice. */
static int ad7418_read_value(struct i2c_client *client, u8 reg)
{
	if (reg == AD7418_REG_CONF || reg == AD7418_REG_CONF2 )
		return i2c_smbus_read_byte_data(client, reg);
	else
		return swab16(i2c_smbus_read_word_data(client, reg));
}

/* All registers are word-sized, except for the configuration register.
   AD7418 uses a high-byte first convention, which is exactly opposite to
   the usual practice. */
static int ad7418_write_value(struct i2c_client *client, u8 reg, u16 value)
{
	if (reg == AD7418_REG_CONF || reg == AD7418_REG_CONF2 )
		return i2c_smbus_write_byte_data(client, reg, value);
	else
		return i2c_smbus_write_word_data(client, reg, swab16(value));
}

static void ad7418_init_client(struct i2c_client *client)
{
	int reg;

	/* Enable if in shutdown mode */
	reg = ad7418_read_value(client, AD7418_REG_CONF);
	if (reg >= 0 && (reg & 0x01))
		ad7418_write_value(client, AD7418_REG_CONF, reg & 0xfe);
}

static struct ad7418_data *ad7418_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ad7418_data *data = i2c_get_clientdata(client);

	down(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
	    || !data->valid) {
		dev_dbg(&client->dev, "Starting ad7418 update\n");

    data->temp_input = ad7418_read_value(client, AD7418_REG_TEMP);
		data->temp_max = ad7418_read_value(client, AD7418_REG_TEMP_OS);
		data->temp_hyst = ad7418_read_value(client, AD7418_REG_TEMP_HYST);
		data->volt = ad7418_read_value(client, AD7418_REG_VOLT) >> 6;
		data->last_updated = jiffies;
		data->valid = 1;
	}

	up(&data->update_lock);

	return data;
}

static int __init sensors_ad7418_init(void)
{
	return i2c_add_driver(&ad7418_driver);
}

static void __exit sensors_ad7418_exit(void)
{
	i2c_del_driver(&ad7418_driver);
}

MODULE_AUTHOR("Frodo Looijaard <frodol@dds.nl>");
MODULE_DESCRIPTION("AD7418 driver");
MODULE_LICENSE("GPL");

module_init(sensors_ad7418_init);
module_exit(sensors_ad7418_exit);
