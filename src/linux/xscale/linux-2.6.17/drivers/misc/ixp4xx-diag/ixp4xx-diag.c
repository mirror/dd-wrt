/*
 * drivers/misc/ixp4xx-led/ixp4xx-led.c
 *
 */
 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/sysctl.h>
#include <linux/platform_device.h>

#include <asm/hardware.h>	/* Pick up IXP4xx-specific bits */

static unsigned int diag = 0;
static unsigned int reset = 0;

static void
set_leds( struct ixp4xx_diag_led_pins *gpio )
{
	int i;
	for( i = 0; i < gpio->led_pin_count; i++ )
		gpio_line_set(gpio->led_pins[i], 0 != (diag & (1<<i)));
}

static int 
proc_diag(ctl_table *ctl, int write, struct file * filp,
				void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int r;
	r = proc_dointvec(ctl, write, filp, buffer, lenp, ppos);
	if (write && !r) {
		set_leds( (struct ixp4xx_diag_led_pins *)ctl->extra1 );
	}
	return r;
}

static int 
proc_reset(ctl_table *ctl, int write, struct file * filp,
				void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct ixp4xx_diag_led_pins *gpio = ctl->extra1;
	if( gpio->reset_pin != ~0u ) {
		int val;
		gpio_line_get(gpio->reset_pin, &val);
		reset = (gpio->reset_polarity && val) ||
		       (!gpio->reset_polarity && !val);
	} else {
		reset = 0;
	}

	return proc_dointvec(ctl, write, filp, buffer, lenp, ppos);
}

static struct ctl_table_header *diag_sysctl_header;

static ctl_table sys_diag[] = {
    {
        ctl_name: 2000,
        procname: "diag",
        data: &diag,
        maxlen: sizeof(diag),
        mode: 0644,
		proc_handler: proc_diag,
  		extra1: NULL
    },
    {
		ctl_name: 2001,
		procname: "reset",
		data: &reset,
		maxlen: sizeof(reset),
		mode: 0444,
		proc_handler: proc_reset,
		extra1: NULL
	},
	{ 0 }
};


static int ixp4xx_led_probe(struct device *dev)
{
	int i;
	struct platform_device *plat_dev = to_platform_device(dev);
	struct ixp4xx_diag_led_pins *gpio = plat_dev->dev.platform_data;

	for( i = 0; i < gpio->led_pin_count; i++ )
	    gpio_line_config(gpio->led_pins[i], IXP4XX_GPIO_OUT);
	if( gpio->reset_pin != ~0u )
	    gpio_line_config(gpio->reset_pin, IXP4XX_GPIO_IN);

	set_leds( gpio );
	
	dev_set_drvdata( &plat_dev->dev, gpio );

	sys_diag[0].extra1 = gpio;
	sys_diag[1].extra1 = gpio;
	diag_sysctl_header = register_sysctl_table(sys_diag, 0);

	return 0;
}

static int ixp4xx_led_remove(struct device *dev)
{
	if( diag_sysctl_header )
		unregister_sysctl_table(diag_sysctl_header);
    return 0;
}

static struct device_driver ixp4xx_diag_driver = {
	.name		= "IXP4XX-DIAG",
	.bus		= &platform_bus_type,
	.probe		= ixp4xx_led_probe,
	.remove		= ixp4xx_led_remove,
};

static int __init ixp4xx_led_init(void)
{
	return driver_register(&ixp4xx_diag_driver);
}

static void __exit ixp4xx_led_exit(void)
{
	driver_unregister(&ixp4xx_diag_driver);
}

module_init(ixp4xx_led_init);
module_exit(ixp4xx_led_exit);

MODULE_DESCRIPTION("IXP4XX LED/Button Diag driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Barnabas Kalman <ba...@sednet.hu>");
