#include <linux/init.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <asm/rb/rb100.h>

static void mr_led_set_user(struct led_classdev *led_cdev,
			       enum led_brightness brightness)
{
	rb100_set_port_led2(MR_PORT_USER_LED, brightness);
}

static struct led_classdev mr_led = {
       .name = "user-led",
       .brightness_set = mr_led_set_user,
};

static int mr_led_probe(struct platform_device *pdev)
{
	return led_classdev_register(&pdev->dev, &mr_led);
}

static struct platform_driver mr_led_driver = {
	.probe	= mr_led_probe,
	.driver	= {
		.name = "mr-led",
		.owner = THIS_MODULE,
	},
};

static int __init mr_led_init(void)
{
	return platform_driver_register(&mr_led_driver);
}

static void __exit mr_led_exit(void)
{
	platform_driver_unregister(&mr_led_driver);
}

module_init(mr_led_init);
module_exit(mr_led_exit);
