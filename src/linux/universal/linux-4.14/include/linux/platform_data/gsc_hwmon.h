/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _GSC_HWMON_H
#define _GSC_HWMON_H

enum gsc_hwmon_type {
	type_temperature,
	type_voltage,
	type_voltage_raw,
	type_fan,
};

/**
 * struct gsc_hwmon_channel - configuration parameters
 * @reg:  I2C register offset
 * @type: channel type
 * @name: channel name
 * @voffset: voltage offset (mV)
 * @vdiv: voltage divider array (2 resistor values in ohms)
 */
struct gsc_hwmon_channel {
	unsigned int reg;
	unsigned int type;
	const char *name;
	unsigned int voffset;
	unsigned int vdiv[2];
};

/**
 * struct gsc_hwmon_platform_data - platform data for gsc_hwmon driver
 * @channels:	pointer to array of gsc_hwmon_channel structures
 *		describing channels
 * @nchannels:	number of elements in @channels array
 * @vreference: voltage reference (mV)
 * @resolution: ADC resolution
 * @fan_base: register base for FAN controller
 */
struct gsc_hwmon_platform_data {
	const struct gsc_hwmon_channel *channels;
	int nchannels;
	unsigned int resolution;
	unsigned int vreference;
	unsigned int fan_base;
};

#endif
