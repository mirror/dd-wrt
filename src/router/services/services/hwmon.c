/*
 * hwmon.c
 *
 * Copyright (C) 2009 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> /* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <net/route.h> /* AhMan March 18 2005 */
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>
#include <syslog.h>
#include <services.h>

#if defined(HAVE_CPUTEMP) && !defined(HAVE_BCMMODERN)
#ifndef HAVE_LAGUNA

#ifdef HAVE_GATEWORX
#define TEMP_PATH "/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028"
#define TEMP2_PATH "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028"
// #define TEMP_PATH "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028"
#define TEMP_PREFIX "temp"
#define TEMP2_PREFIX "temp1"
#define TEMP_MUL 100
#elif HAVE_UNIWIP
#define TEMP_PREFIX "temp1"
#define TEMP_PATH "/sys/bus/i2c/devices/0-0049"
#define TEMP_MUL 1000
#else
#ifdef HAVE_X86
#define TEMP_PATH "/sys/devices/platform/i2c-1/1-0048"
#else
#define TEMP_PATH "/sys/devices/platform/i2c-0/0-0048"
#endif
#define TEMP_PREFIX "temp1"
#define TEMP_MUL 1000
#endif

void stop_hwmon(void)
{
}

void start_hwmon(void)
{
	int temp_max = nvram_geti("hwmon_temp_max") * TEMP_MUL;
	int temp_hyst = nvram_geti("hwmon_temp_hyst") * TEMP_MUL;

	sysprintf("/bin/echo %d > %s/%s_max", temp_max, TEMP_PATH, TEMP_PREFIX);
	sysprintf("/bin/echo %d > %s/%s_max_hyst", temp_hyst, TEMP_PATH, TEMP_PREFIX);
#ifdef TEMP2_PATH
	sysprintf("/bin/echo %d > %s/%s_max", temp_max, TEMP2_PATH, TEMP2_PREFIX);
	sysprintf("/bin/echo %d > %s/%s_max_hyst", temp_hyst, TEMP2_PATH, TEMP2_PREFIX);
#endif
	dd_loginfo("hwmon", "successfully started");
}
#endif
#endif
