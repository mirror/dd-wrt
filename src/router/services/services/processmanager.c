/*
 * processmanager.c
 *
 * Copyright (C) 2010 Sebastian Gottschall <gottschall@dd-wrt.com>
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

int stop_process(char *name, char *desc)
{
	if (!desc)
		desc = name;
	if (pidof(name) > 0) {
		dd_syslog(LOG_INFO, "%s : %s successfully stopped\n", name, desc);
		killall(name, SIGTERM);
		int deadcounter = 20;
		while (pidof(name) > 0 && deadcounter--) {
			usleep(100 * 1000);
		}
		if (pidof(name) > 0) {
			dd_syslog(LOG_INFO, "%s : %s hanging, send SIGKILL\n", name, desc);
			killall(name, SIGKILL);
		}
		return 1;
	}
	return 0;
}
