/*
 * zabbix.c
 *
 * Copyright (C) 2013 Richard Schneidt
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

#ifdef HAVE_ZABBIX
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>

void start_zabbix(void)
{
	if (!nvram_matchi("zabbix_enable", 1))
		return;
	if (pidof("zabbix_agentd") > 0) {
		//syslog(LOG_INFO, "dlna : minidlna already running\n");
	} else {
		dd_logstart("zabbix", eval("sh", "/etc/config/zabbix.startup"));
	}

	return;
}

void stop_zabbix(void)
{
	stop_process("zabbix_agentd", "daemon");
	/* kill it once again since stop_process does not close all instances */
	if (pidof("zabbix_agentd") > 0) {
		eval("kill", "-9", pidof("zabbix_agentd"));
	}

	return;
}
#endif
