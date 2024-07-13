/*
 * cron.c
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
#ifdef HAVE_CRON
#ifndef HAVE_MICRO
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
#include <time.h>

void stop_cron(void);

void start_cron(void)
{
	int ret = 0;
	struct stat buf;
	FILE *fp;

	if (nvram_matchi("cron_enable", 0))
		return;

	sysprintf("grep -q crontabs /etc/passwd || echo \"crontabs:*:0:0:Contab User,,,:/var:/bin/false\" >> /etc/passwd");

	stop_cron();

	update_timezone();

	/*
	 * Create cron's database directory 
	 */
	if (stat("/var/spool", &buf) != 0) {
		mkdir("/var/spool", 0700);
		mkdir("/var/spool/cron", 0700);
	}
	mkdir("/tmp/cron.d", 0700);

	buf_to_file("/tmp/cron.d/check_ps", "*/2 * * * * root /sbin/check_ps\n");
	/*
	 * pppoe reconnect 
	 */
	unlink("/tmp/cron.d/pppoe_reconnect"); // may change, so we reconnect
	if (nvram_matchi("reconnect_enable", 1)) {
		fp = fopen("/tmp/cron.d/pppoe_reconnect", "w");
		fprintf(fp, "%s %s * * * root /sbin/service wan_redial start\n", nvram_safe_get("reconnect_minutes"),
			nvram_safe_get("reconnect_hours"));
		fclose(fp);
	}
	/*
	 * reboot scheduler 
	 */
	unlink("/tmp/cron.d/check_schedules");
	if (nvram_matchi("schedule_enable", 1) && nvram_matchi("schedule_hour_time", 2)) {
		fp = fopen("/tmp/cron.d/check_schedules", "w");
		fprintf(fp, "%s %s * * %s root /sbin/service run_rc_shutdown start; /sbin/reboot\n",
			nvram_safe_get("schedule_minutes"), nvram_safe_get("schedule_hours"), nvram_safe_get("schedule_weekdays"));
		fclose(fp);
	}
	/*
	 * ppp_peer.db backup
	 */
	unlink("/tmp/cron.d/ppp_peer_backup"); //
	if (jffs_mounted()) {
		fp = fopen("/tmp/cron.d/ppp_peer_backup", "w");
		if (nvram_matchi("pppoeserver_enabled", 1))
			fprintf(fp, "1 0,12 * * * root /bin/cp /tmp/pppoe_peer.db /jffs/etc/freeradius/\n");
		if (nvram_matchi("pptpd_enable", 1))
			fprintf(fp, "1 0,12 * * * root /bin/cp /tmp/pptp_peer.db /jffs/etc/freeradius/\n");
		fclose(fp);
	}

	/*
	 * Additional cron jobs 
	 */
	unlink("/tmp/cron.d/cron_jobs");

	if (nvram_invmatch("cron_jobs", "")) {
		fp = fopen("/tmp/cron.d/cron_jobs", "w");
		fwritenvram("cron_jobs", fp);
		fprintf(fp, "\n"); // extra new line at the end
		fclose(fp);
	}
#ifdef HAVE_HOTSPOT
	struct tm *currtime;
	long tloc;

	time(&tloc);
	currtime = localtime(&tloc);

	unlink("/tmp/cron.d/hotss_checkalive");

	if (nvram_matchi("hotss_enable", 1)) {
		fp = fopen("/tmp/cron.d/hotss_checkalive", "w");

		fprintf(fp,
			"%d * * * * root /usr/bin/wget http://tech.hotspotsystem.com/up.php?mac=`nvram get wl0_hwaddr|sed s/:/-/g`\\&nasid=%s_%s\\&os_date=`nvram get os_date|sed s/\" \"/-/g`\\&install=2\\&uptime=`uptime|sed s/\" \"/\\%%20/g|sed s/:/\\%%3A/g|sed s/,/\\%%2C/g`  -O /tmp/lastup.html\n",
			(currtime->tm_min + 3) % 60, nvram_safe_get("hotss_operatorid"), nvram_safe_get("hotss_locationid"));

		fclose(fp);
	}
#endif
#ifdef HAVE_RAID
	fp = fopen("/tmp/cron.d/fscheck", "w");
	fprintf(fp, "0 3 1 * * root /sbin/service fscheck main\n");
	fclose(fp);

#endif

	/*
	 * Custom cron files 
	 */
	eval("cp", "-af", "/tmp/mycron.d/*", "/tmp/cron.d/");
	eval("cp", "-af", "/jffs/mycron.d/*", "/tmp/cron.d/");
	eval("cp", "-af", "/mmc/mycron.d/*", "/tmp/cron.d/");

	cprintf("starting cron\n");

	log_eval("cron");

	cprintf("done\n");
	return;
}

void stop_cron(void)
{
	stop_process("cron", "daemon");
	cprintf("done\n");
	return;
}
#endif
#endif
