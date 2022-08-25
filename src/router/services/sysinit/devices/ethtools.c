/*
 * ethtools.c
 *
 * Copyright (C) 2014 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 * 
 * detects ethernet adapters and loads the drivers
 */

static void setSwitchLED(int gpio, int portmask)
{
	sysprintf("echo switch0 > /sys/class/leds/generic_%d/trigger", gpio);
	sysprintf("echo 0x%x > /sys/class/leds/generic_%d/port_mask", portmask, gpio);
}

static void setEthLED(int gpio, char *eth)
{
	sysprintf("echo netdev > /sys/class/leds/generic_%d/trigger", gpio);
	sysprintf("echo %s > /sys/class/leds/generic_%d/device_name", eth, gpio);
	sysprintf("echo \"link tx rx\" > /sys/class/leds/generic_%d/mode", gpio);
}

static void setEthLinkLED(int gpio, char *eth)
{
	sysprintf("echo netdev > /sys/class/leds/generic_%d/trigger", gpio);
	sysprintf("echo %s > /sys/class/leds/generic_%d/device_name", eth, gpio);
	sysprintf("echo \"link\" > /sys/class/leds/generic_%d/mode", gpio);
}
