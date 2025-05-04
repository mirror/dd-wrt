/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Register definitions the RTL8231 GPIO and LED expander chip
 */

#ifndef __LINUX_MFD_RTL8231_H
#define __LINUX_MFD_RTL8231_H

#include <linux/bits.h>

/*
 * Registers addresses are 5 bit, values are 16 bit
 * Also define a duplicated range of virtual addresses, to enable
 * different read/write behaviour on the GPIO data registers
 */
#define RTL8231_BITS_VAL		16
#define RTL8231_BITS_REG		5

/* Chip control */
#define RTL8231_REG_FUNC0		0x00
#define RTL8231_FUNC0_SCAN_MODE		BIT(0)
#define RTL8231_FUNC0_SCAN_SINGLE	0
#define RTL8231_FUNC0_SCAN_BICOLOR	BIT(0)

#define RTL8231_REG_FUNC1		0x01
#define RTL8231_FUNC1_READY_CODE_VALUE	0x37
#define RTL8231_FUNC1_READY_CODE_MASK	GENMASK(9, 4)
#define RTL8231_FUNC1_DEBOUNCE_MASK	GENMASK(15, 10)

/* Pin control */
#define RTL8231_REG_PIN_MODE0		0x02
#define RTL8231_REG_PIN_MODE1		0x03

#define RTL8231_PIN_MODE_LED		0
#define RTL8231_PIN_MODE_GPIO		1

/* Pin high config: pin and GPIO control for pins 32-26 */
#define RTL8231_REG_PIN_HI_CFG		0x04
#define RTL8231_PIN_HI_CFG_MODE_MASK	GENMASK(4, 0)
#define RTL8231_PIN_HI_CFG_DIR_MASK	GENMASK(9, 5)
#define RTL8231_PIN_HI_CFG_INV_MASK	GENMASK(14, 10)
#define RTL8231_PIN_HI_CFG_SOFT_RESET	BIT(15)

/* GPIO control registers */
#define RTL8231_REG_GPIO_DIR0		0x05
#define RTL8231_REG_GPIO_DIR1		0x06
#define RTL8231_REG_GPIO_INVERT0	0x07
#define RTL8231_REG_GPIO_INVERT1	0x08

#define RTL8231_GPIO_DIR_IN		1
#define RTL8231_GPIO_DIR_OUT		0

/*
 * GPIO data registers
 * Only the output data can be written to these registers, and only the input
 * data can be read.
 */
#define RTL8231_REG_GPIO_DATA0		0x1c
#define RTL8231_REG_GPIO_DATA1		0x1d
#define RTL8231_REG_GPIO_DATA2		0x1e
#define RTL8231_PIN_HI_DATA_MASK	GENMASK(4, 0)

/* LED control base registers */
#define RTL8231_REG_LED0_BASE		0x09
#define RTL8231_REG_LED1_BASE		0x10
#define RTL8231_REG_LED2_BASE		0x17
#define RTL8231_REG_LED_END		0x1b

#define RTL8231_REG_COUNT		0x1f

#endif /* __LINUX_MFD_RTL8231_H */
