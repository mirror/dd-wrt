/*
 * radiotimer.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> /* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <net/route.h> /* AhMan March 18 2005 */
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>
#include <syslog.h>
#include <services.h>

void start_radio_timer(void)
{
#ifndef HAVE_NOWIFI
	if (nvram_matchi("radio0_timer_enable", 0) && nvram_matchi("radio1_timer_enable", 0) &&
	    nvram_matchi("radio2_timer_enable", 0))
		return;
#ifdef HAVE_MADWIFI
	if (nvram_match("wlan0_net_mode", "disabled") && nvram_match("wlan1_net_mode", "disabled") &&
	    nvram_match("wlan2_net_mode", "disabled"))
#else
	if (nvram_match("wl0_net_mode", "disabled") && nvram_match("wl1_net_mode", "disabled") &&
	    nvram_match("wl2_net_mode", "disabled"))
#endif
		return;

	char *argv[] = { "radio_timer", NULL };
	_log_evalpid(argv, NULL, 0, NULL);

	cprintf("done");
	return;
#endif
}

void stop_radio_timer(void)
{
#ifndef HAVE_NOWIFI
	stop_process("radio_timer", "daemon");
	cprintf("done\n");

	return;
#endif
}
