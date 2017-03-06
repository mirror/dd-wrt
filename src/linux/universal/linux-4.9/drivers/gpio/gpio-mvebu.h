/*
 * Interface between MVEBU GPIO driver and PWM driver for GPIO pins
 *
 * Copyright (C) 2015, Andrew Lunn <andrew@lunn.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef MVEBU_GPIO_PWM_H
#define MVEBU_GPIO_PWM_H

#define BLINK_ON_DURATION	0x0
#define BLINK_OFF_DURATION	0x4
#define GPIO_BLINK_CNT_SELECT	0x0020

struct mvebu_pwm {
	void __iomem	*membase;
	unsigned long	 clk_rate;
	bool		 used;
	unsigned	 pin;
	struct pwm_chip	 chip;
	int		 id;
	spinlock_t	 lock;

	/* Used to preserve GPIO/PWM registers across suspend /
	 * resume */
	u32		 blink_select;
	u32		 blink_on_duration;
	u32		 blink_off_duration;
};

struct mvebu_gpio_chip {
	struct gpio_chip   chip;
	spinlock_t	   lock;
	void __iomem	  *membase;
	void __iomem	  *percpu_membase;
	int		   irqbase;
	struct irq_domain *domain;
	int		   soc_variant;
	struct clk	  *clk;
#ifdef CONFIG_PWM
	struct mvebu_pwm pwm;
#endif
	/* Used to preserve GPIO registers across suspend/resume */
	u32		   out_reg;
	u32		   io_conf_reg;
	u32		   blink_en_reg;
	u32		   in_pol_reg;
	u32		   edge_mask_regs[4];
	u32		   level_mask_regs[4];
};

void mvebu_gpio_blink(struct gpio_chip *chip, unsigned pin, int value);

#ifdef CONFIG_PWM
int mvebu_pwm_probe(struct platform_device *pdev,
		    struct mvebu_gpio_chip *mvchip,
		    int id);
void mvebu_pwm_suspend(struct mvebu_gpio_chip *mvchip);
void mvebu_pwm_resume(struct mvebu_gpio_chip *mvchip);
#else
int mvebu_pwm_probe(struct platform_device *pdev,
		    struct mvebu_gpio_chip *mvchip,
		    int id)
{
	return 0;
}

void mvebu_pwm_suspend(struct mvebu_gpio_chip *mvchip)
{
}

void mvebu_pwm_resume(struct mvebu_gpio_chip *mvchip)
{
}
#endif
#endif
