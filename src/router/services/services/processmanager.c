/*
 * processmanager.c
 *
 * Copyright (C) 2010 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

static int _stop_process(char *name, char *desc, int hard, int maxtime)
{
	if (!desc)
		desc = name;
	if (debug_ready() && nvram_matchi("console_debug", 1))
		dd_loginfo(name, "%s trying to stop", desc);

	if (pidof(name) > 0) {
		if (hard)
			killall(name, SIGKILL);
		else
			killall(name, SIGTERM);
		int deadcounter = maxtime * 10;
		while (pidof(name) > 0 && deadcounter--) {
			usleep(100 * 1000);
		}
		if (pidof(name) > 0) {
			dd_loginfo(name, "%s hanging, send SIGKILL", desc);
			killall(name, SIGKILL);
		}
		dd_loginfo(name, "%s successfully stopped", desc);
		return 1;
	}
	return 0;
}

int reload_process(char *name)
{
	int pid = pidof(name);
	if (pid > 0) {
		kill(pid, SIGHUP);
		dd_loginfo(name, "config reloaded");
		return 0;
	}
	return -1;
}

int stop_process(char *name, char *desc)
{
	return _stop_process(name, desc, 0, 2);
}

int stop_process_timeout(char *name, char *desc, int timeout)
{
	return _stop_process(name, desc, 0, timeout);
}

int stop_process_hard(char *name, char *desc)
{
	return _stop_process(name, desc, 1, 2);
}

void network_delay(char *service)
{
	FILE *first = fopen("/tmp/firstrun", "rb");
	if (!first) {
		dd_loginfo(service, "wait for network init");
		sleep(10); // first run. wait for network init (need a better solution for this)
		first = fopen("/tmp/firstrun", "wb");
		putc('r', first);
		fclose(first);
	} else
		fclose(first);
}
