/*
 * nstx.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_NSTX
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void stop_nstxd(void)
{
	if (pidof("nstxd") > 0) {
		dd_syslog(LOG_INFO,
			  "nstxd : nstx daemon successfully stopped\n");
		killall("nstxd", SIGTERM);
	}
}

void start_nstxd(void)
{
	if (nvram_match("nstxd_enable", "1")) {
		stop_nstxd();
		eval("nstxd");
		dd_syslog(LOG_INFO, "nstxd daemon successfully started\n");
	}
}

#endif
