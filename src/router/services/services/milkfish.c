/*
 * milkfish.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_MILKFISH
#include <sys/stat.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <md5.h>
#include <services.h>

void start_milkfish_boot(void);

void stop_milkfish(void)
{
	if (stop_process("rtpproxy", "rtpproxy") || stop_process("openser", "daemon")) {
		eval("milkfish_services", "stop");
	}
}

void start_milkfish(void)
{
	if (nvram_matchi("milkfish_enabled", 1)) {
		start_milkfish_boot();
		dd_logstart("milkfish", eval("/etc/config/milkfish.netup"));
	}
}

void start_milkfish_boot(void)
{
	md5_ctx_t MD;

	if (strlen(nvram_safe_get("milkfish_routerid")) != 32) {
		unsigned char hash[32];
		char et0[18];
		getLANMac(et0);
		if (!*et0)
			strcpy(et0, nvram_safe_get("et0macaddr_safe"));

		dd_md5_begin(&MD);
		dd_md5_hash(et0, 17, &MD);
		dd_md5_end((unsigned char *)hash, &MD);
		char request[32];
		int i;

		for (i = 0; i < 16; i++)
			sprintf(&request[2 * i], "%02x", hash[i]);
		nvram_set("milkfish_routerid", request);
		nvram_seti("need_commit", 1);
	}
	// Start the milkfish services
	eval("milkfish_services", "start");
	// dbtext module setup
	mkdir("/var/openser", 0700);
	mkdir("/var/openser/dbtext", 0700);
	eval("cp", "/etc/openser/dbtext/aliases", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/location", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/subscriber", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/version", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/uri", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/grp", "/var/openser/dbtext/");

	// restore dbtext parts which may have been saved into nvram
	eval("milkfish_services", "sipdb", "restorenvdd");

	// firewall configuration in networking/firewall.c
	return;
}
#endif
