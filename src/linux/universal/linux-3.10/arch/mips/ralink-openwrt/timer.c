/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>
#include <linux/leds.h>
#include <linux/slab.h>

#include <asm/mach-ralink-openwrt/ralink_regs.h>

#define TIMER_REG_TMRSTAT		0x00
#define TIMER_REG_TMR0LOAD		0x10
#define TIMER_REG_TMR0CTL		0x18

#define TMRSTAT_TMR0INT			BIT(0)

#define TMR0CTL_ENABLE			BIT(7)
#define TMR0CTL_MODE_PERIODIC		BIT(4)
#define TMR0CTL_PRESCALER		2
#define TMR0CTL_PRESCALE_VAL		(0xf - TMR0CTL_PRESCALER)
#define TMR0CTL_PRESCALE_DIV		(65536 / BIT(TMR0CTL_PRESCALER))

struct rt_timer_gpio {
	struct list_head	list;
	struct led_classdev	*led;
};

struct rt_timer {
	struct device		*dev;
	void __iomem		*membase;
	int			irq;

	unsigned long		timer_freq;
	unsigned long		timer_div;

	struct list_head	gpios;
	struct led_trigger	led_trigger;
	unsigned int		duty_cycle;
	unsigned int		duty;

	unsigned int		fade;
	unsigned int		fade_min;
	unsigned int		fade_max;
	unsigned int		fade_speed;
	unsigned int		fade_dir;
	unsigned int		fade_count;
};

static inline void rt_timer_w32(struct rt_timer *rt, u8 reg, u32 val)
{
	__raw_writel(val, rt->membase + reg);
}

static inline u32 rt_timer_r32(struct rt_timer *rt, u8 reg)
{
	return __raw_readl(rt->membase + reg);
}

static irqreturn_t rt_timer_irq(int irq, void *_rt)
{
	struct rt_timer *rt =  (struct rt_timer *) _rt;
	struct rt_timer_gpio *gpio;
	unsigned int val;

	if (rt->fade && (rt->fade_count++ > rt->fade_speed)) {
		rt->fade_count = 0;
		if (rt->duty_cycle <= rt->fade_min)
			rt->fade_dir = 1;
		else if (rt->duty_cycle >= rt->fade_max)
			rt->fade_dir = 0;

		if (rt->fade_dir)
			rt->duty_cycle += 1;
		else
			rt->duty_cycle -= 1;

	}

	val = rt->timer_freq / rt->timer_div;
	if (rt->duty)
		val *= rt->duty_cycle;
	else
		val *= (100 - rt->duty_cycle);
	val /= 100;

	if (!list_empty(&rt->gpios))
		list_for_each_entry(gpio, &rt->gpios, list)
			led_set_brightness(gpio->led, !!rt->duty);

	rt->duty = !rt->duty;

	rt_timer_w32(rt, TIMER_REG_TMR0LOAD, val + 1);
	rt_timer_w32(rt, TIMER_REG_TMRSTAT, TMRSTAT_TMR0INT);

	return IRQ_HANDLED;
}

static int rt_timer_request(struct rt_timer *rt)
{
	int err = devm_request_irq(rt->dev, rt->irq, rt_timer_irq,
					IRQF_DISABLED, dev_name(rt->dev), rt);
	if (err) {
		dev_err(rt->dev, "failed to request irq\n");
	} else {
		u32 t = TMR0CTL_MODE_PERIODIC | TMR0CTL_PRESCALE_VAL;
		rt_timer_w32(rt, TIMER_REG_TMR0CTL, t);
	}
	return err;
}

static void rt_timer_free(struct rt_timer *rt)
{
	free_irq(rt->irq, rt);
}

static int rt_timer_config(struct rt_timer *rt, unsigned long divisor)
{
	if (rt->timer_freq < divisor)
		rt->timer_div = rt->timer_freq;
	else
		rt->timer_div = divisor;

	return 0;
}

static int rt_timer_enable(struct rt_timer *rt)
{
	u32 t;

	rt_timer_w32(rt, TIMER_REG_TMR0LOAD, rt->timer_freq / rt->timer_div);

	t = rt_timer_r32(rt, TIMER_REG_TMR0CTL);
	t |= TMR0CTL_ENABLE;
	rt_timer_w32(rt, TIMER_REG_TMR0CTL, t);

	return 0;
}

static void rt_timer_disable(struct rt_timer *rt)
{
	u32 t;

	t = rt_timer_r32(rt, TIMER_REG_TMR0CTL);
	t &= ~TMR0CTL_ENABLE;
	rt_timer_w32(rt, TIMER_REG_TMR0CTL, t);
}

static ssize_t led_fade_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct rt_timer *rt = container_of(led_cdev->trigger, struct rt_timer, led_trigger);

	return sprintf(buf, "speed: %d, min: %d, max: %d\n", rt->fade_speed, rt->fade_min, rt->fade_max);
}

static ssize_t led_fade_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct rt_timer *rt = container_of(led_cdev->trigger, struct rt_timer, led_trigger);
	unsigned int speed = 0, min = 0, max = 0;
	ssize_t ret = -EINVAL;

	ret = sscanf(buf, "%u %u %u", &speed, &min, &max);

	if (ret == 3) {
		rt->fade_speed = speed;
		rt->fade_min = min;
		rt->fade_max = max;
		rt->fade = 1;
	} else {
		rt->fade = 0;
	}

	return size;
}

static DEVICE_ATTR(fade, 0644, led_fade_show, led_fade_store);

static ssize_t led_duty_cycle_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct rt_timer *rt = container_of(led_cdev->trigger, struct rt_timer, led_trigger);

	return sprintf(buf, "%u\n", rt->duty_cycle);
}

static ssize_t led_duty_cycle_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct rt_timer *rt = container_of(led_cdev->trigger, struct rt_timer, led_trigger);
	unsigned long state;
	ssize_t ret = -EINVAL;

	ret = kstrtoul(buf, 10, &state);
	if (ret)
		return ret;

	if (state <= 100)
		rt->duty_cycle = state;
	else
		rt->duty_cycle = 100;

	rt->fade = 0;

	return size;
}

static DEVICE_ATTR(duty_cycle, 0644, led_duty_cycle_show, led_duty_cycle_store);

static void rt_timer_trig_activate(struct led_classdev *led_cdev)
{
	struct rt_timer *rt = container_of(led_cdev->trigger, struct rt_timer, led_trigger);
	struct rt_timer_gpio *gpio_data;
	int rc;

	led_cdev->trigger_data = NULL;
	gpio_data = kzalloc(sizeof(*gpio_data), GFP_KERNEL);
	if (!gpio_data)
		return;

	rc = device_create_file(led_cdev->dev, &dev_attr_duty_cycle);
	if (rc)
		goto err_gpio;
	rc = device_create_file(led_cdev->dev, &dev_attr_fade);
	if (rc)
		goto err_out_duty_cycle;

	led_cdev->activated = true;
	led_cdev->trigger_data = gpio_data;
	gpio_data->led = led_cdev;
	list_add(&gpio_data->list, &rt->gpios);
	led_cdev->trigger_data = gpio_data;
	rt_timer_enable(rt);
	return;

err_out_duty_cycle:
	device_remove_file(led_cdev->dev, &dev_attr_duty_cycle);

err_gpio:
	kfree(gpio_data);
}

static void rt_timer_trig_deactivate(struct led_classdev *led_cdev)
{
	struct rt_timer *rt = container_of(led_cdev->trigger, struct rt_timer, led_trigger);
	struct rt_timer_gpio *gpio_data = (struct rt_timer_gpio*) led_cdev->trigger_data;

	if (led_cdev->activated) {
		device_remove_file(led_cdev->dev, &dev_attr_duty_cycle);
		device_remove_file(led_cdev->dev, &dev_attr_fade);
		led_cdev->activated = false;
	}

	list_del(&gpio_data->list);
	rt_timer_disable(rt);
	led_set_brightness(led_cdev, LED_OFF);
}

static int rt_timer_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	const __be32 *divisor;
	struct rt_timer *rt;
	struct clk *clk;
	int ret;

	rt = devm_kzalloc(&pdev->dev, sizeof(*rt), GFP_KERNEL);
	if (!rt) {
		dev_err(&pdev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	rt->irq = platform_get_irq(pdev, 0);
	if (!rt->irq) {
		dev_err(&pdev->dev, "failed to load irq\n");
		return -ENOENT;
	}

	rt->membase = devm_request_and_ioremap(&pdev->dev, res);
	if (IS_ERR(rt->membase))
		return PTR_ERR(rt->membase);

	clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "failed get clock rate\n");
		return PTR_ERR(clk);
	}

	rt->timer_freq = clk_get_rate(clk) / TMR0CTL_PRESCALE_DIV;
	if (!rt->timer_freq)
		return -EINVAL;

	rt->duty_cycle = 100;
	rt->dev = &pdev->dev;
	platform_set_drvdata(pdev, rt);

	ret = rt_timer_request(rt);
	if (ret)
		return ret;

	divisor = of_get_property(pdev->dev.of_node, "ralink,divisor", NULL);
	if (divisor)
		rt_timer_config(rt, be32_to_cpu(*divisor));
	else
		rt_timer_config(rt, 200);

	rt->led_trigger.name = "pwmtimer",
	rt->led_trigger.activate = rt_timer_trig_activate,
	rt->led_trigger.deactivate = rt_timer_trig_deactivate,

	ret = led_trigger_register(&rt->led_trigger);
	if (ret)
		return ret;

	INIT_LIST_HEAD(&rt->gpios);

	dev_info(&pdev->dev, "maximum frequncy is %luHz\n", rt->timer_freq);

	return 0;
}

static int rt_timer_remove(struct platform_device *pdev)
{
	struct rt_timer *rt = platform_get_drvdata(pdev);

	led_trigger_unregister(&rt->led_trigger);
	rt_timer_disable(rt);
	rt_timer_free(rt);

	return 0;
}

static const struct of_device_id rt_timer_match[] = {
	{ .compatible = "ralink,rt2880-timer" },
	{},
};
MODULE_DEVICE_TABLE(of, rt_timer_match);

static struct platform_driver rt_timer_driver = {
	.probe = rt_timer_probe,
	.remove = rt_timer_remove,
	.driver = {
		.name		= "rt-timer",
		.owner          = THIS_MODULE,
		.of_match_table	= rt_timer_match
	},
};

module_platform_driver(rt_timer_driver);

MODULE_DESCRIPTION("Ralink RT2880 timer / pseudo pwm");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org");
MODULE_LICENSE("GPL");
