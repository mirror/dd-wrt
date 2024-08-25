/*
 * wpswatcher.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <rc.h>

#include <cy_conf.h>
#include <utils.h>
#include <nvparse.h>

static int wpswatcher_main(int argc, char **argv)
{
	/* 
	 * Run it in the background 
	 */
	switch (fork()) {
	case -1:
		// can't fork
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		// fork ok
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}
	int timeout = atoi(argv[1]);
	while (timeout) {
		FILE *fp = fopen("/tmp/.wpsdone", "rb");
		if (fp) {
			killall("ledtool", SIGKILL);
			nvram_seti("wps_status", 1);
			nvram_async_commit();
			unlink("/tmp/.wpsdone");
			fclose(fp);
			led_control(LED_SES, LED_ON);
			break;
		}
		timeout--;
		sleep(1);
	}
	if (!timeout) {
		killall("ledtool", SIGKILL);
		nvram_seti("wps_status", 1);
		nvram_async_commit();
		eval("ledtool", "1800", "3");
	}
	return 0;
} // end main
