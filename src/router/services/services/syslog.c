/*
 * syslog.c
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

#ifdef HAVE_SYSLOG
void start_syslog(void)
{
	int ret1 = 0, ret2 = 0;

	if (!nvram_invmatch("syslogd_enable", "0"))
		return;

	if (strlen(nvram_safe_get("syslogd_rem_ip")) > 0)
		ret1 = eval("syslogd", "-L -R", nvram_safe_get("syslogd_rem_ip"));
	else
		ret1 = eval("syslogd", "-L");

	dd_syslog(LOG_INFO, "syslogd : syslog daemon successfully started\n");
	ret2 = eval("klogd");
	dd_syslog(LOG_INFO, "klogd : klog daemon successfully started\n");

	return;
}

void stop_syslog(void)
{
	stop_process("klogd", "kernel log daemon");
	stop_process("syslogd", "syslog daemon");
	return;
}
#endif
