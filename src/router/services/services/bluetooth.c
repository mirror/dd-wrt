/*
 * bluetooth.c
 *
 * Copyright (C) 2009 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <ddnvram.h>
#include <shutils.h>
#include <services.h>

void start_bluetooth(void)
{
	char path[64];
	char conffile[64];
	if (!nvram_matchi("bt_enable", 1)) {
		stop_bluetooth();
		return;
	}
	eval("modprobe", "bluetooth");
	eval("modprobe", "rfcomm");
	eval("modprobe", "bnep");
	eval("modprobe", "hidp");
	eval("modprobe", "hci_uart");
	eval("modprobe", "btusb");
	system("/usr/lib/bluetooth/bluetoothd&");
	int brand = getRouterBrand();
	switch (brand) {
	case ROUTER_LINKSYS_MR7350:
	case ROUTER_LINKSYS_MR7500:
		eval("hciattach", "/dev/ttyMSM1", "bcsp", "115200", "noflow");
		break;
	case ROUTER_LINKSYS_MX8500:
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
		eval("hciattach", "-s", "115200", "/dev/ttyMSM1", "any", "115200");
		break;
	}
	return;
}

void restart_bluetooth(void)
{
	start_bluetooth();
}

void stop_bluetooth(void)
{
	stop_process("bluetooth", "hciattach");

	return;
}
