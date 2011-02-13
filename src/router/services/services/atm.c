/*
 * atm.c
 *
 * Copyright (C) 2011 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_DANUBE
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void start_atm(void)
{
	if (!nvram_match("wan_proto", "pppoa"))
		sysprintf("br2684ctl -b -c 0 -e 0 -p 1 -a 0.%s.%s",nvram_safe_get("vpi"),nvram_safe_get("vci"));
	dd_syslog(LOG_INFO, "ATM : DSL Modem interface created\n");

	cprintf("done\n");
	return;
}

void stop_atm(void)
{
	stop_process("br2684ctl", "ATM: DSL Modem interface removed");
	cprintf("done\n");
	return;
}
#endif
