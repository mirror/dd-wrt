/*
 * Device driver for MMC/SD cards driven over a GPIO bus.
 *
 * Copyright (c) 2008 Michael Buesch
 *
 * Licensed under the GNU/GPL version 2.
 */
#ifndef LINUX_GPIOMMC_H_
#define LINUX_GPIOMMC_H_

#include <linux/types.h>


#define GPIOMMC_MAX_NAMELEN		15
#define GPIOMMC_MAX_NAMELEN_STR		__stringify(GPIOMMC_MAX_NAMELEN)

/**
 * struct gpiommc_pins - Hardware pin assignments
 *
 * @gpio_di: The GPIO number of the DATA IN pin
 * @gpio_do: The GPIO number of the DATA OUT pin
 * @gpio_clk: The GPIO number of the CLOCK pin
 * @gpio_cs: The GPIO number of the CHIPSELECT pin
 * @cs_activelow: If true, the chip is considered selected if @gpio_cs is low.
 */
struct gpiommc_pins {
	unsigned int gpio_di;
	unsigned int gpio_do;
	unsigned int gpio_clk;
	unsigned int gpio_cs;
	bool cs_activelow;
};

/**
 * struct gpiommc_platform_data - Platform data for a MMC-over-SPI-GPIO device.
 *
 * @name: The unique name string of the device.
 * @pins: The hardware pin assignments.
 * @mode: The hardware mode. This is either SPI_MODE_0,
 *        SPI_MODE_1, SPI_MODE_2 or SPI_MODE_3. See the SPI documentation.
 * @no_spi_delay: Do not use delays in the lowlevel SPI bitbanging code.
 *                This is not standards compliant, but may be required for some
 *                embedded machines to gain reasonable speed.
 * @max_bus_speed: The maximum speed of the SPI bus, in Hertz.
 */
struct gpiommc_platform_data {
	char name[GPIOMMC_MAX_NAMELEN + 1];
	struct gpiommc_pins pins;
	u8 mode;
	bool no_spi_delay;
	unsigned int max_bus_speed;
};

/**
 * GPIOMMC_PLATDEV_NAME - The platform device name string.
 *
 * The name string that has to be used for platform_device_alloc
 * when allocating a gpiommc device.
 */
#define GPIOMMC_PLATDEV_NAME	"gpiommc"

/**
 * gpiommc_next_id - Get another platform device ID number.
 *
 * This returns the next platform device ID number that has to be used
 * for platform_device_alloc. The ID is opaque and should not be used for
 * anything else.
 */
int gpiommc_next_id(void);

#endif /* LINUX_GPIOMMC_H_ */
