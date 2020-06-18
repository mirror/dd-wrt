// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Antaira Technologies custom i2c gpio expander
 *
 * Author: Björn Geschka <bjoern@newmedia-net.de>
 */

/*
 * this is a custom gpiochip over i2c driver, exporting the following gpios:
 * gpio 200 0x00 0x01 low power LED
 * gpio 201 0x00 0x02 medium power LED
 * gpio 202 0x00 0x04 high power LED
 * gpio 203 0x00 0x08 N/C
 * 
 * gpio 204 0x01 0x01 N/C
 * gpio 205 0x01 0x02 N/C
 * gpio 206 0x01 0x04 N/C
 * gpio 207 0x01 0x08 N/C
 * 
 * gpio 208 0x02 0x01 N/C
 * gpio 209 0x02 0x02 N/C
 * gpio 210 0x02 0x04 N/C
 * gpio 211 0x02 0x08 N/C
 * 
 * gpio 212 0x03 0x01 DIAG LED
 * gpio 213 0x03 0x02 N/C
 * gpio 214 0x03 0x04 N/C
 * gpio 215 0x03 0x08 Buzzer
*/


#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gpio.h>

#define MODNAME "antairagpio"
#define GPIO_BASE 200
#define GPIO_NUM 16

#define LOG(form, ...) \
	printk(KERN_INFO "["MODNAME"]" "%s:%d " form "\n", \
			__func__, __LINE__, ##__VA_ARGS__)

struct antaira_gpiochip {
	struct i2c_client *client;
	struct gpio_chip chip;
};

static inline struct antaira_gpiochip *to_antaira_gpiochip(struct gpio_chip *gc)
{
	return container_of(gc, struct antaira_gpiochip, chip);
}

static int antairagpio_get(struct gpio_chip *gc, unsigned offset){
	struct antaira_gpiochip *antaira_dev = to_antaira_gpiochip(gc);
	unsigned byte = offset / 4;
	int bit = (offset % 4);
	int cur = i2c_smbus_read_byte_data(antaira_dev->client, byte);

	if(((cur) & (1<<(bit)))) return 1;
	return 0;
}
static void antairagpio_set(struct gpio_chip *gc, unsigned offset, int value){
	struct antaira_gpiochip *antaira_dev = to_antaira_gpiochip(gc);
	unsigned byte = offset / 4;
	int bit = (offset % 4);
	int cur = i2c_smbus_read_byte_data(antaira_dev->client, byte);
	int new = cur;

	if(value == 0)
		((new) &= ~(1<<(bit)));
	else
		((new) |=  (1<<(bit)));

	if(new == cur) return;
	i2c_smbus_write_byte_data(antaira_dev->client, byte, new);
}

static int antairagpio_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct antaira_gpiochip *antaira_dev;
	antaira_dev = devm_kzalloc(&client->dev, sizeof(*antaira_dev), GFP_KERNEL);
	if (!antaira_dev)
		return -ENOMEM;
	
	antaira_dev->chip.label = client->name;
	antaira_dev->chip.base = GPIO_BASE;
	antaira_dev->chip.parent = &client->dev;
	antaira_dev->chip.owner = THIS_MODULE;
	antaira_dev->chip.ngpio = GPIO_NUM;
	antaira_dev->chip.can_sleep = 1;
	antaira_dev->chip.get = antairagpio_get;
	antaira_dev->chip.set = antairagpio_set;
	antaira_dev->client = client;

	i2c_set_clientdata(client, antaira_dev);

	LOG("initial reset");
	i2c_smbus_write_byte_data(antaira_dev->client, 0x00, 0x00);
	i2c_smbus_write_byte_data(antaira_dev->client, 0x03, 0x00);

	return gpiochip_add(&antaira_dev->chip);
}

static int antairagpio_remove(struct i2c_client *client)
{
	struct antaira_gpiochip *antaira_dev;
	antaira_dev = i2c_get_clientdata(client);
	gpiochip_remove(&antaira_dev->chip);
	return 0;
}

static const struct i2c_device_id antairagpio_id[] = {
	{ MODNAME , 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, antairagpio_id);

static struct i2c_driver antairagpio_driver = {
	.driver = {
		.name = MODNAME,
	},
	.probe = antairagpio_probe,
	.remove = antairagpio_remove,
	.id_table = antairagpio_id,
};

static int __init antairagpio_init(void)
{
	return i2c_add_driver(&antairagpio_driver);
}
subsys_initcall(antairagpio_init);

static void __exit antairagpio_exit(void)
{
	i2c_del_driver(&antairagpio_driver);
}
module_exit(antairagpio_exit);

MODULE_AUTHOR("Björn Geschka");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(MODNAME" i2c GPIO-Expander");
