#include <linux/init.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <asm/rb/cr.h>

static void cr_led_set_user(struct led_classdev *led_cdev,
			       enum led_brightness brightness)
{
	if (brightness)
		CR_GPOUT() |= CR_GPIO_ULED;
	else
		CR_GPOUT() &= ~CR_GPIO_ULED;
}

static struct led_classdev cr_led = {
       .name = "user-led",
       .brightness_set = cr_led_set_user,
};

static int cr_led_probe(struct platform_device *pdev)
{
	return led_classdev_register(&pdev->dev, &cr_led);
}

static struct platform_driver cr_led_driver = {
	.probe	= cr_led_probe,
	.driver	= {
		.name = "cr-led",
		.owner = THIS_MODULE,
	},
};

static int __init cr_led_init(void)
{
	return platform_driver_register(&cr_led_driver);
}

static void __exit cr_led_exit(void)
{
	platform_driver_unregister(&cr_led_driver);
}

module_init(cr_led_init);
module_exit(cr_led_exit);
