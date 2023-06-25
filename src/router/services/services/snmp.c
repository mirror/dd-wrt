/*
 * snmp.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#ifdef HAVE_SNMP

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <services.h>
#include "snmp.h"

char *snmp_deps(void)
{
	return "snmpd_enable snmpd_syslocation snmpd_syscontact snmpd_sysname snmpd_rocommunity snmpd_rwcommunity";
}

char *snmp_proc(void)
{
	return "snmpd";
}

void start_snmp(void)
{
	char path[64];
	char *snmpd_argv[] = { "snmpd", "-c", getdefaultconfig("snmp", path, sizeof(path), "snmpd.conf"), NULL };
	FILE *fp = NULL;

	if (!nvram_invmatchi("snmpd_enable", 0)) {
		stop_snmp();
		return;
	}

	fp = fopen(getdefaultconfig("snmp", path, sizeof(path), "snmpd.conf"), "w");
	if (NULL == fp)
		return;

	if (*(nvram_safe_get("snmpd_syslocation")))
		fprintf(fp, "syslocation %s\n", nvram_safe_get("snmpd_syslocation"));
	if (*(nvram_safe_get("snmpd_syscontact")))
		fprintf(fp, "syscontact %s\n", nvram_safe_get("snmpd_syscontact"));
	if (*(nvram_safe_get("snmpd_sysname")))
		fprintf(fp, "sysname %s\n", nvram_safe_get("snmpd_sysname"));
	if (*(nvram_safe_get("snmpd_rocommunity")))
		fprintf(fp, "rocommunity %s\n", nvram_safe_get("snmpd_rocommunity"));
	if (*(nvram_safe_get("snmpd_rwcommunity")))
		fprintf(fp, "rwcommunity %s\n", nvram_safe_get("snmpd_rwcommunity"));
	fprintf(fp, "sysservices 9\n");
#ifdef HAVE_RAYTRONIK
	fprintf(fp, "pass_persist .1.3.6.1.4.1.41404.255 /etc/config/wl_snmpd.sh\n");
#else
	fprintf(fp, "pass_persist .1.3.6.1.4.1.2021.255 /etc/wl_snmpd.sh\n");
#endif
	fclose(fp);
	if (reload_process("snmpd")) {
		_log_evalpid(snmpd_argv, NULL, 0, NULL);
	}
	return;
}

void restart_snmp(void)
{
	start_snmp();
}

void stop_snmp(void)
{
	stop_process("snmpd", "daemon");
	nvram_delstates(snmp_deps());
	return;
}
#endif
