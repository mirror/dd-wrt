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


	mkdir("/tmp/var/run/dbus", 0744);
	FILE *fp = fopen("/tmp/avahi-dbus.conf", "wb");
	fprintf(fp,
		"<!DOCTYPE busconfig PUBLIC\n" //
		"\"-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN\"\n" //
		"\"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">\n" //
		"<busconfig>\n" //
		"\n" //
		"<!-- Only root or user nobody can own the Avahi service this needs some work -->\n" //
		"<policy user=\"nobody\">\n" //
		"<allow own=\"*\"/>\n" //
		"</policy>\n" //
		"<policy user=\"root\">\n" //
		"<allow own=\"*\"/>\n" //
		"</policy>\n" //
		"\n" //
		"<!-- Allow anyone to invoke methods on Avahi server, except SetHostName -->\n" //
		"<policy context=\"default\">\n" //
		"<allow send_destination=\"*\"/>\n" //
		"<allow receive_sender=\"*\"/>\n" //
		"\n" //
		"<deny send_destination=\"org.freedesktop.Avahi\"\n" //
		"send_interface=\"org.freedesktop.Avahi.Server\" send_member=\"SetHostName\"/>\n" //
		"</policy>\n" //
		"\n" //
		"<listen>unix:path=/tmp/var/run/dbus/system_bus_socket</listen>\n" //
		"\n" //
		"<!-- Allow everything, including access to SetHostName to users of the group \"nobody\" -->\n" //
		"<policy group=\"nobody\">\n" //
		"<allow send_destination=\"*\"/>\n" //
		"<allow receive_sender=\"*\"/>\n" //
		"</policy>\n" //
		"<policy user=\"root\">\n" //
		"<allow send_destination=\"*\"/>\n" //
		"<allow receive_sender=\"*\"/>\n" //
		"</policy>\n" //
		"</busconfig>\n");
	fclose(fp);

	if (pidof("dbus-daemon") > 0) {
		dd_loginfo("dbus-daemon", "dbus-daemon already running");
	} else {
		snprintf(conffile, sizeof(conffile), "--config-file=%s",
			 getdefaultconfig("mdns", path, sizeof(path), "avahi-dbus.conf"));
		log_eval("dbus-launch", conffile);
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
