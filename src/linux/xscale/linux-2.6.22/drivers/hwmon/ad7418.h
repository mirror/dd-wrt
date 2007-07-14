/*
    ad7418.h - Part of lm_sensors, Linux kernel modules for hardware
             monitoring
    Copyright (c) 2003 Mark M. Hoffman <mhoffman@lightlink.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
    This file contains common code for encoding/decoding AD7418 type
    temperature readings, which are emulated by many of the chips
    we support.  As the user is unlikely to load more than one driver
    which contains this code, we don't worry about the wasted space.
*/

#include <linux/hwmon.h>

/* straight from the datasheet */
#define AD7418_TEMP_MIN (-40000)
#define AD7418_TEMP_MAX 125000

/* TEMP: 0.001C/bit (-40C to +125C)
   REG: (0.25C/bit, two's complement) << 6 */
static inline u16 AD7418_TEMP_TO_REG(int temp)
{
	int ntemp = SENSORS_LIMIT(temp, AD7418_TEMP_MIN, AD7418_TEMP_MAX);
	return (u16)((ntemp / 25) << 6);
}

static inline int AD7418_TEMP_FROM_REG(u16 reg)
{
	/* use integer division instead of equivalent right shift to
	   guarantee arithmetic shift and preserve the sign */
	return ((s16)reg / 64) * 25;
}

static inline int ADC_TO_GW2342VOLT(u16 reg)
{
	return (((int)(reg) * 564) / 10);
}
