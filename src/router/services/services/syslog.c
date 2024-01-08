/*
 * syslog.c
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

#ifdef HAVE_SYSLOG
char *syslog_deps(void)
{
	return "syslogd_enable syslogd_rem_ip klogd_enable";
}

char *syslog_proc(void)
{
	return "syslogd";
}

void stop_syslog(void);
void start_syslog(void)
{
	int ret1 = 0, ret2 = 0;
	stop_syslog();
	if (!nvram_invmatchi("syslogd_enable", 0))
		return;
	update_timezone();
	if (nvram_matchi("syslogd_jffs2", 1) && !nvram_matchi("flash_active", 1)) {
		mkdir("/jffs/log", 0700);
		eval("mv", "-f", "/jffs/log/messages", "/jffs/log/messages.old");
		FILE *fp = fopen("/jffs/log/messages", "wb");
		if (!fp)
			goto fallback;
		fclose(fp);
		if (*(nvram_safe_get("syslogd_rem_ip")))
			log_eval("syslogd", "-Z", "-b", "5", "-L", "-R", nvram_safe_get("syslogd_rem_ip"), "-O",
				 "/jffs/log/messages");
		else
			log_eval("syslogd", "-Z", "-b", "5", "-L", "-O", "/jffs/log/messages");
	} else {
fallback:;
		if (*(nvram_safe_get("syslogd_rem_ip")))
			log_eval("syslogd", "-Z", "-L", "-R", nvram_safe_get("syslogd_rem_ip"));
		else
			log_eval("syslogd", "-Z", "-L");
	}
	if (!nvram_invmatchi("klogd_enable", 0))
		return;
	log_eval("klogd");

	return;
}

void stop_syslog(void)
{
	stop_process("klogd", "kernel log daemon");
	stop_process("syslogd", "syslog daemon");
	nvram_delstates(syslog_deps());
	return;
}
#endif
