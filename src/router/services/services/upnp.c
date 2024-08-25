/*
 * upnp.c
 *
 * Copyright (C) 2007 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_UPNP
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_upnp(void);

void start_upnp(void)
{
	char wan_if_buffer[33];
	if (nvram_match("wan_proto", "disabled"))
		return;
	char *wan_ifname = safe_get_wan_face(wan_if_buffer);
	int ret;

	if (nvram_matchi("upnp_enable", 0)) {
		stop_upnp();
		return;
	}
	/*
	 * Make sure its not running first 
	 */
	ret = killall("upnp", SIGUSR1);
	if (ret != 0) {
		log_eval("upnp", "-D", "-W", wan_ifname);
	}

	cprintf("done\n");
	return;
}

void stop_upnp(void)
{
	stop_process("upnp",
		     "daemon"); // we dont need to take care about SIGUSR1 anymore
	unlink("/tmp/bcmupnp.pid"); // remove pid file if it exists, otherwise upnp wont restart if killed with SIGKILL
	return;
}

int reinit_upnp(void)
{
	int ret = eval("killall", "-USR1", "upnp");

	return ret;
}

#endif
