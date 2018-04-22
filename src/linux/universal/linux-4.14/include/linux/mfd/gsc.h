/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2018 Gateworks Corporation
 */
#ifndef __LINUX_MFD_GSC_H_
#define __LINUX_MFD_GSC_H_

/* Device Addresses */
#define GSC_MISC	0x20
#define GSC_UPDATE	0x21
#define GSC_GPIO	0x23
#define GSC_HWMON	0x29
#define GSC_EEPROM0	0x50
#define GSC_EEPROM1	0x51
#define GSC_EEPROM2	0x52
#define GSC_EEPROM3	0x53
#define GSC_RTC		0x68

/* Register offsets */
#define GSC_CTRL_0	0x00
#define GSC_CTRL_1	0x01
#define GSC_TIME	0x02
#define GSC_TIME_ADD	0x06
#define GSC_IRQ_STATUS	0x0A
#define GSC_IRQ_ENABLE	0x0B
#define GSC_FW_CRC	0x0C
#define GSC_FW_VER	0x0E
#define GSC_WP		0x0F

/* Bit definitions */
#define GSC_CTRL_0_PB_HARD_RESET	0
#define GSC_CTRL_0_PB_CLEAR_SECURE_KEY	1
#define GSC_CTRL_0_PB_SOFT_POWER_DOWN	2
#define GSC_CTRL_0_PB_BOOT_ALTERNATE	3
#define GSC_CTRL_0_PERFORM_CRC		4
#define GSC_CTRL_0_TAMPER_DETECT	5
#define GSC_CTRL_0_SWITCH_HOLD		6

#define GSC_CTRL_1_SLEEP_ENABLE		0
#define GSC_CTRL_1_ACTIVATE_SLEEP	1
#define GSC_CTRL_1_LATCH_SLEEP_ADD	2
#define GSC_CTRL_1_SLEEP_NOWAKEPB	3
#define GSC_CTRL_1_WDT_TIME		4
#define GSC_CTRL_1_WDT_ENABLE		5
#define GSC_CTRL_1_SWITCH_BOOT_ENABLE	6
#define GSC_CTRL_1_SWITCH_BOOT_CLEAR	7

#define GSC_IRQ_PB			0
#define GSC_IRQ_KEY_ERASED		1
#define GSC_IRQ_EEPROM_WP		2
#define GSC_IRQ_RESV			3
#define GSC_IRQ_GPIO			4
#define GSC_IRQ_TAMPER			5
#define GSC_IRQ_WDT_TIMEOUT		6
#define GSC_IRQ_SWITCH_HOLD		7

enum gsc_type {
	gsc_v1 = 1,
	gsc_v2 = 2,
	gsc_v3 = 3,
};

struct gsc_dev {
	struct device *dev;

	struct i2c_client *i2c;		/* 0x20: interrupt controller, WDT */
	struct i2c_client *i2c_hwmon;	/* 0x29: hwmon, fan controller */

	struct regmap *regmap;
	struct regmap *regmap_hwmon;
	struct regmap_irq_chip_data *irq_chip_data;

	int irq;
	int type;
	unsigned int fwver;
	unsigned short fwcrc;
};

#endif /* __LINUX_MFD_GSC_H_ */
