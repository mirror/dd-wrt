/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006 Felix Fietkau <nbd@openwrt.org>
 */

/*
 * Support for GPIO -- General Purpose Input/Output Pins
 * XXX: should be rewritten
 */

#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include "ar531xlnx.h"

/* GPIO Interrupt Support */

/* Turn on the specified AR531X_GPIO_IRQ interrupt */
static unsigned int
ar531x_gpio_intr_startup(unsigned int irq)
{
	ar531x_gpio_intr_enable(irq);

	return 0;
}

/* Turn off the specified AR531X_GPIO_IRQ interrupt */
static void
ar531x_gpio_intr_shutdown(unsigned int irq)
{
	ar531x_gpio_intr_disable(irq);
}

u32 gpioIntMask = 0;

static void ar531x_gpio_intr_set_enabled(unsigned int gpio, int enabled)
{
	u32 reg;
	int intnum = 0;
	int intlevel = 2;
   
	reg = sysRegRead(AR5315_GPIO_CR);
	reg &= ~(GPIO_CR_M(gpio));
	reg |= GPIO_CR_I(gpio);
	sysRegWrite(AR5315_GPIO_CR, reg);
	(void)sysRegRead(AR5315_GPIO_CR); /* flush write to hardware */

	reg = sysRegRead(AR5315_GPIO_INT);

	reg &= ~(GPIO_INT_M(intnum));
	reg &= ~(GPIO_INT_LVL_M(intnum));

	if (enabled) {
		reg |= GPIO_INT_LVL(intlevel, intnum);
		reg |= GPIO_INT(gpio, intnum);
	}

	sysRegWrite(AR5315_GPIO_INT, reg);
	(void)sysRegRead(AR5315_GPIO_INT); /* flush write to hardware */
}


/* Enable the specified AR531X_GPIO_IRQ interrupt */
static void
ar531x_gpio_intr_enable(unsigned int irq)
{
	int gpio = irq - AR531X_GPIO_IRQ_BASE;

	gpioIntMask |= (1<<gpio);
	ar531x_gpio_intr_set_enabled(irq, 1);
}

/* Disable the specified AR531X_GPIO_IRQ interrupt */
static void
ar531x_gpio_intr_disable(unsigned int irq)
{
	int gpio = irq - AR531X_GPIO_IRQ_BASE;

	gpioIntMask &= (1<<gpio);
	ar531x_gpio_intr_set_enabled(irq, 0);
}


static void
ar531x_gpio_intr_end(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS)))
		ar531x_gpio_intr_enable(irq);
}

int ar531x_gpio_irq_base;

struct hw_interrupt_type ar531x_gpio_intr_controller = {
	.typename = "AR531X GPIO",
	.startup = ar531x_gpio_intr_startup,
	.shutdown = ar531x_gpio_intr_shutdown,
	.enable = ar531x_gpio_intr_enable,
	.disable = ar531x_gpio_intr_disable,
	.ack = ar531x_gpio_intr_disable,
	.end = ar531x_gpio_intr_end,
};

void
ar531x_gpio_intr_init(int irq_base)
{
	int i;

	for (i = irq_base; i < irq_base + AR531X_GPIO_IRQ_COUNT; i++) {
		irq_desc[i].status = IRQ_DISABLED;
		irq_desc[i].action = NULL;
		irq_desc[i].depth = 1;
		irq_desc[i].chip = &ar531x_gpio_intr_controller;
	}

	ar531x_gpio_irq_base = irq_base;
}


