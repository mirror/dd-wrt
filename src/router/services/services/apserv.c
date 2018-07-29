/*
 * apserv.c
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
#ifdef HAVE_AP_SERV
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_apserv(void);

void start_apserv(void)
{
	char *wan_ifname = get_wan_face();
	int ret;

	if (nvram_matchi("apserv_enable", 0)) {
		stop_apserv();
		return;
	}
	/*
	 * Make sure its not running first 
	 */
	ret = killall("ap_serv", SIGUSR1);
	ret = eval("ap_serv", "-i", nvram_safe_get("lan_ifname"));
	dd_loginfo("ap_serv", "daemon successfully started\n");
	cprintf("done\n");
	return;
}

void stop_apserv(void)
{
	stop_process("ap_serv", "buffalo discovery daemon");
	return;
}

#endif
