/*
 * aoss.c
 *
 * Copyright (C) 2010 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_AOSS
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void stop_aoss(void);

void start_aoss(void)
{
	int ret;

	if (nvram_match("aoss_enable", "0")) {
		stop_aoss();
		return;
	}

	killall("aoss", SIGTERM);
	ret = eval("aoss", "-i", nvram_safe_get("lan_ifname"), "-m", "ap");
	dd_syslog(LOG_INFO, "aoss : aoss daemon successfully started\n");
	cprintf("done\n");
	return;
}

void stop_aoss(void)
{
	stop_process("aoss", "buffalo aoss daemon");
	return;
}

#endif
