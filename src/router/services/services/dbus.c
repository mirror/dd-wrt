/*
 * dbus.c
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

void start_dbus(void)
{
	char path[64];
	char conffile[64];
	if (!nvram_matchi("mdns_enable", 1) && !nvram_matchi("bt_enable", 1)) {
		stop_dbus();
		return;
	}
	mkdir("/tmp/var/run/dbus",0744);
	mkdir("/tmp/var/lib/dbus",0744);
	if (pidof("dbus-daemon") > 0) {
		dd_loginfo("dbus-daemon", "dbus-daemon already running");
	} else {
		log_eval("dbus-launch");
	}
	return;
}

void restart_dbus(void)
{
	start_dbus();
}

void stop_dbus(void)
{
	stop_process("dbus-daemon", "dbus-daemon");
	return;
}
