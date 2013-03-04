/*
 * arch/arm/mach-cns3xxx/include/mach/gpio.h
 *
 * CNS3xxx GPIO wrappers for arch-neutral GPIO calls
 *
 * Copyright 2011 Gateworks Corporation
 *		  Chris Lang <clang@gateworks.com>
 *
 * Based on IXP implementation by Milan Svoboda <msvoboda@ra.rockwell.com>
 * Based on PXA implementation by Philipp Zabel <philipp.zabel@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef __ASM_ARCH_CNS3XXX_GPIO_H
#define __ASM_ARCH_CNS3XXX_GPIO_H

#include <linux/kernel.h>
#include <linux/io.h>
#include <asm-generic/gpio.h>			/* cansleep wrappers */
#include <mach/cns3xxx.h>

#define NR_BUILTIN_GPIO 64

#define CNS3XXX_GPIO_IN    0x0
#define CNS3XXX_GPIO_OUT   0x1

#define CNS3XXX_GPIO_LO   0
#define CNS3XXX_GPIO_HI   1

#define CNS3XXX_GPIO_OUTPUT         0x00
#define CNS3XXX_GPIO_INPUT          0x04
#define CNS3XXX_GPIO_DIR            0x08
#define CNS3XXX_GPIO_SET            0x10
#define CNS3XXX_GPIO_CLEAR          0x14


static inline void gpio_line_config(u8 line, u32 direction)
{
	u32 reg;
	if (direction) {
		if (line < 32) {
			reg = __raw_readl((void __iomem *)(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_DIR));
			reg |= (1 << line);
			__raw_writel(reg, (void __iomem *)(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_DIR));
		} else {
			reg = __raw_readl((void __iomem *)(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_DIR));
			reg |= (1 << (line - 32));
			__raw_writel(reg, (void __iomem *)(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_DIR));		
		}
	} else {
		if (line < 32) {
			reg = __raw_readl((void __iomem *)(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_DIR));
			reg &= ~(1 << line);
			__raw_writel(reg, (void __iomem *)(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_DIR));
		} else {
			reg = __raw_readl((void __iomem *)(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_DIR));
			reg &= ~(1 << (line - 32));
			__raw_writel(reg, (void __iomem *)(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_DIR));		
		}
	}
}


static inline void gpio_line_get(u8 line, int *value)
{
	if (line < 32)
		*value = ((__raw_readl((void __iomem *)(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_INPUT)) >> line) & 0x1);
	else
		*value = ((__raw_readl((void __iomem *)(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_INPUT)) >> (line - 32)) & 0x1);
}

static inline void gpio_line_set(u8 line, int value)
{
	if (line < 32) {
		if (value)
			__raw_writel((1 << line), (void __iomem *)(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_SET));
		else
			__raw_writel((1 << line), (void __iomem *)(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_CLEAR));
	} else {
		if (value)
			__raw_writel((1 << line), (void __iomem *)(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_SET));
		else
			__raw_writel((1 << line), (void __iomem *)(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_CLEAR));
	}
}

static inline int gpio_get_value(unsigned gpio)
{
	if (gpio < NR_BUILTIN_GPIO)
	{
		int value;
		gpio_line_get(gpio, &value);
		return value;
	}
	else
		return __gpio_get_value(gpio);
}

static inline void gpio_set_value(unsigned gpio, int value)
{
	if (gpio < NR_BUILTIN_GPIO)
		gpio_line_set(gpio, value);
	else
		__gpio_set_value(gpio, value);
}

#define gpio_cansleep __gpio_cansleep

extern int irq_to_gpio(int gpio);

#endif
