/*
 * cron.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <sys/ioctl.h>		/* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
#include <net/route.h>		/* AhMan March 18 2005 */
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


void start_cron(void)
{
	int ret = 0;
	struct stat buf;
	FILE *fp;

	if (nvram_match("cron_enable", "0"))
		return;

	if (pidof("cron") > 0)	//cron already running
		return;

	/*
	 * Create cron's database directory 
	 */
	if (stat("/var/spool", &buf) != 0) {
		mkdir("/var/spool", 0700);
		mkdir("/var/spool/cron", 0700);
	}
	mkdir("/tmp/cron.d", 0700);

	buf_to_file("/tmp/cron.d/check_ps",
		    "*/2 * * * * root /sbin/check_ps\n");
	/*
	 * pppoe reconnect 
	 */
	unlink("/tmp/cron.d/pppoe_reconnect");
	if (nvram_match("reconnect_enable", "1")) {

		fp = fopen("/tmp/cron.d/pppoe_reconnect", "w");
		fprintf(fp, "%s %s * * * root /usr/bin/killall pppd\n",
			nvram_safe_get("reconnect_minutes"),
			nvram_safe_get("reconnect_hours"));
		fclose(fp);
	}
	/*
	 * reboot scheduler 
	 */
	unlink("/tmp/cron.d/check_schedules");
	if (nvram_match("schedule_enable", "1")
	    && nvram_match("schedule_hour_time", "2")) {

		fp = fopen("/tmp/cron.d/check_schedules", "w");
		fprintf(fp, "%s %s * * %s root /sbin/reboot\n",
			nvram_safe_get("schedule_minutes"),
			nvram_safe_get("schedule_hours"),
			nvram_safe_get("schedule_weekdays"));
		fclose(fp);
	}

	/*
	 * Additional cron jobs 
	 */
	unlink("/tmp/cron.d/cron_jobs");

	if (nvram_invmatch("cron_jobs", "")) {

		fp = fopen("/tmp/cron.d/cron_jobs", "w");
		fwritenvram("cron_jobs", fp);
		fprintf(fp, "\n");	// extra new line at the end
		fclose(fp);
	}

	/*
	 * Custom cron files 
	 */
	eval("cp", "-af", "/tmp/mycron.d/*", "/tmp/cron.d/");
	eval("cp", "-af", "/jffs/mycron.d/*", "/tmp/cron.d/");
	eval("cp", "-af", "/mmc/mycron.d/*", "/tmp/cron.d/");

	cprintf("starting cron\n");
	if (pidof("cron") > 0)	//cron already running
		return;
	ret = eval("cron");
	dd_syslog(LOG_INFO, "cron : cron daemon successfully started\n");

	cprintf("done\n");
	return;
}

void stop_cron(void)
{

	if (pidof("cron") > 0) {
		dd_syslog(LOG_INFO,
			  "cron : cron daemon successfully stopped\n");
		killall("cron", SIGKILL);
		eval("rm", "-rf", "/tmp/cron.d");
	}
	cprintf("done\n");
	return;
}
