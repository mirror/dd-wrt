/*
 *  AR71xx SoC routines
 *
 *  Copyright (C) 2007 Atheros 
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/proc_fs.h>
#include <asm/types.h>
#include <asm/irq.h>

#include "ar7100.h"

/*
 * GPIO interrupt stuff
 */
typedef enum {
	INT_TYPE_EDGE,
	INT_TYPE_LEVEL,
} ar7100_gpio_int_type_t;

typedef enum {
	INT_POL_ACTIVE_LOW,
	INT_POL_ACTIVE_HIGH,
} ar7100_gpio_int_pol_t;

/* 
** Simple Config stuff
*/

#if !defined(IRQ_NONE)
#define IRQ_NONE
#define IRQ_HANDLED
#endif				/* !defined(IRQ_NONE) */

typedef irqreturn_t(*sc_callback_t) (int, void *, struct pt_regs *);

static sc_callback_t registered_cb = NULL;
static void *cb_arg;
static struct proc_dir_entry *simple_config_entry = NULL;
static struct proc_dir_entry *simulate_push_button_entry = NULL;
static struct proc_dir_entry *tricolor_led_entry = NULL;

void ar7100_gpio_config_int(int gpio, ar7100_gpio_int_type_t type, ar7100_gpio_int_pol_t polarity)
{
	u32 val;

	/*
	 * allow edge sensitive/rising edge too
	 */
	if (type == INT_TYPE_LEVEL) {
		/* level sensitive */
		ar7100_reg_rmw_set(AR7100_GPIO_INT_TYPE, (1 << gpio));
	} else {
		/* edge triggered */
		val = ar7100_reg_rd(AR7100_GPIO_INT_TYPE);
		val &= ~(1 << gpio);
		ar7100_reg_wr(AR7100_GPIO_INT_TYPE, val);
	}

	if (polarity == INT_POL_ACTIVE_HIGH) {
		ar7100_reg_rmw_set(AR7100_GPIO_INT_POLARITY, (1 << gpio));
	} else {
		val = ar7100_reg_rd(AR7100_GPIO_INT_POLARITY);
		val &= ~(1 << gpio);
		ar7100_reg_wr(AR7100_GPIO_INT_POLARITY, val);
	}

	ar7100_reg_rmw_set(AR7100_GPIO_INT_ENABLE, (1 << gpio));
}

void ar7100_gpio_config_output(int gpio)
{
	ar7100_reg_rmw_set(AR7100_GPIO_OE, (1 << gpio));
}

void ar7100_gpio_config_input(int gpio)
{
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, (1 << gpio));
}

void ar7100_gpio_out_val(int gpio, int val)
{
	if (val & 0x1) {
		ar7100_reg_rmw_set(AR7100_GPIO_OUT, (1 << gpio));
	} else {
		ar7100_reg_rmw_clear(AR7100_GPIO_OUT, (1 << gpio));
	}
}

int ar7100_gpio_in_val(int gpio)
{
	return ((1 << gpio) & (ar7100_reg_rd(AR7100_GPIO_IN)));
}

static void ar7100_gpio_intr_enable(unsigned int irq)
{
	ar7100_reg_rmw_set(AR7100_GPIO_INT_MASK, (1 << (irq - AR7100_GPIO_IRQ_BASE)));
}

static void ar7100_gpio_intr_disable(unsigned int irq)
{
	ar7100_reg_rmw_clear(AR7100_GPIO_INT_MASK, (1 << (irq - AR7100_GPIO_IRQ_BASE)));
}

static unsigned int ar7100_gpio_intr_startup(unsigned int irq)
{
	ar7100_gpio_intr_enable(irq);
	return 0;
}

static void ar7100_gpio_intr_shutdown(unsigned int irq)
{
	ar7100_gpio_intr_disable(irq);
}

static void ar7100_gpio_intr_ack(unsigned int irq)
{
	ar7100_gpio_intr_disable(irq);
}

static void ar7100_gpio_intr_end(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS)))
		ar7100_gpio_intr_enable(irq);
}

static void ar7100_gpio_intr_set_affinity(unsigned int irq, cpumask_t mask)
{
	/* 
	 * Only 1 CPU; ignore affinity request
	 */
}

struct irq_chip ar7100_gpio_irq_chip = {
	.name = "AR7100 GPIO",
	.startup = ar7100_gpio_intr_startup,
	.shutdown = ar7100_gpio_intr_shutdown,
	.enable = ar7100_gpio_intr_enable,
	.disable = ar7100_gpio_intr_disable,

	.ack = ar7100_gpio_intr_ack,

	.end = ar7100_gpio_intr_end,
	.set_affinity = ar7100_gpio_intr_set_affinity,
};

void ar7100_gpio_irq_init(int irq_base)
{
	int i;

	for (i = irq_base; i < irq_base + AR7100_GPIO_IRQ_COUNT; i++) {
		irq_desc[i].status = IRQ_DISABLED;
		irq_desc[i].action = NULL;
		irq_desc[i].depth = 1;
		irq_desc[i].chip = &ar7100_gpio_irq_chip;
	}
}

void register_simple_config_callback(void *callback, void *arg)
{
	registered_cb = (sc_callback_t) callback;
	cb_arg = arg;
}

EXPORT_SYMBOL(register_simple_config_callback);

void unregister_simple_config_callback(void)
{
	registered_cb = NULL;
	cb_arg = NULL;
}

EXPORT_SYMBOL(unregister_simple_config_callback);
#ifdef CONFIG_AR9100
#define JUMPSTART_GPIO    12
#else
#define JUMPSTART_GPIO    3
#endif

#ifdef JUMPSTART
static int ignore_pushbutton = 0;

/*
 * Irq for front panel SW jumpstart switch
 * Connected to XSCALE through GPIO4
 */
irqreturn_t jumpstart_irq(int cpl, void *dev_id, struct pt_regs *regs)
{
	if (ignore_pushbutton) {
		ar7100_gpio_config_int(JUMPSTART_GPIO, INT_TYPE_LEVEL, INT_POL_ACTIVE_HIGH);
		ignore_pushbutton = 0;
		return IRQ_HANDLED;
	}

	ar7100_gpio_config_int(JUMPSTART_GPIO, INT_TYPE_LEVEL, INT_POL_ACTIVE_LOW);
	ignore_pushbutton = 1;

	printk("calling simple_config callback..\n");

	if (registered_cb) {
		return registered_cb(cpl, cb_arg, regs);
	}
	return IRQ_HANDLED;
}
#endif

static int push_button_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	ar7100_gpio_config_input(JUMPSTART_GPIO);
	return sprintf(page, "%d\n", ar7100_gpio_in_val(JUMPSTART_GPIO));
}

static int push_button_write(struct file *file, const char *buf, unsigned long count, void *data)
{
	if (registered_cb) {
		registered_cb(0, cb_arg, 0);
	}
	return count;
}

#define TRICOLOR_LED_GREEN_PIN  5	/* GPIO 5 */
#define TRICOLOR_LED_YELLOW_PIN 4	/* GPIO 4 */
#define OFF 0
#define ON 1

typedef enum {
	LED_STATE_OFF = 0,
	LED_STATE_GREEN = 1,
	LED_STATE_YELLOW = 2,
	LED_STATE_ORANGE = 3,
	LED_STATE_MAX = 4
} led_state_e;

static led_state_e gpio_tricolorled = LED_STATE_OFF;

static int gpio_tricolor_led_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	return sprintf(page, "%d\n", gpio_tricolorled);
}

static int gpio_tricolor_led_write(struct file *file, const char *buf, unsigned long count, void *data)
{
	u_int32_t val, green_led_onoff = 0, yellow_led_onoff = 0;

	if (sscanf(buf, "%d", &val) != 1)
		return -EINVAL;

	if (val >= LED_STATE_MAX)
		return -EINVAL;

	if (val == gpio_tricolorled)
		return count;

	switch (val) {
	case LED_STATE_OFF:
		green_led_onoff = OFF;	/* both LEDs OFF */
		yellow_led_onoff = OFF;
		break;

	case LED_STATE_GREEN:
		green_led_onoff = ON;	/* green ON, Yellow OFF */
		yellow_led_onoff = OFF;
		break;

	case LED_STATE_YELLOW:
		green_led_onoff = OFF;	/* green OFF, Yellow ON */
		yellow_led_onoff = ON;
		break;

	case LED_STATE_ORANGE:
		green_led_onoff = ON;	/* both LEDs ON */
		yellow_led_onoff = ON;
		break;
	}

	ar7100_gpio_out_val(TRICOLOR_LED_GREEN_PIN, green_led_onoff);
	ar7100_gpio_out_val(TRICOLOR_LED_YELLOW_PIN, yellow_led_onoff);
	gpio_tricolorled = val;

	return count;
}

static int create_simple_config_led_proc_entry(void)
{
	if (simple_config_entry != NULL) {
		printk("Already have a proc entry for /proc/simple_config!\n");
		return -ENOENT;
	}

	simple_config_entry = proc_mkdir("simple_config", NULL);
	if (!simple_config_entry)
		return -ENOENT;

	simulate_push_button_entry = create_proc_entry("push_button", 0644, simple_config_entry);
	if (!simulate_push_button_entry)
		return -ENOENT;

	simulate_push_button_entry->write_proc = push_button_write;
	simulate_push_button_entry->read_proc = push_button_read;

	tricolor_led_entry = create_proc_entry("tricolor_led", 0644, simple_config_entry);
	if (!tricolor_led_entry)
		return -ENOENT;

	tricolor_led_entry->write_proc = gpio_tricolor_led_write;
	tricolor_led_entry->read_proc = gpio_tricolor_led_read;

	/* configure gpio as outputs */
	ar7100_gpio_config_output(TRICOLOR_LED_GREEN_PIN);
	ar7100_gpio_config_output(TRICOLOR_LED_YELLOW_PIN);

	/* switch off the led */
	ar7100_gpio_out_val(TRICOLOR_LED_GREEN_PIN, OFF);
	ar7100_gpio_out_val(TRICOLOR_LED_YELLOW_PIN, OFF);

	return 0;
}

int __init pbXX_simple_config_init(void)
{
#ifdef JUMPSTART
	int req;

	/* configure GPIO 3 as level triggered interrupt */
	ar7100_gpio_config_int(JUMPSTART_GPIO, INT_TYPE_LEVEL, INT_POL_ACTIVE_HIGH);

	req = request_irq(AR7100_GPIO_IRQn(JUMPSTART_GPIO), jumpstart_irq, 0, "SW JUMPSTART", NULL);
	if (req != 0) {
		printk(KERN_ERR "unable to request IRQ for SWJUMPSTART GPIO (error %d)\n", req);
	}
#endif

	create_simple_config_led_proc_entry();
	return 0;
}

subsys_initcall(pbXX_simple_config_init);
