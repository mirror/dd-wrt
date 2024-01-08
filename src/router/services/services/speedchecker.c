/*
 * speedchecker.c
 *
 * Copyright (C) 2017 Christian Scheele  <scheele@dd-wrt.com>
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

#ifdef HAVE_SPEEDCHECKER
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
#include <prevision.h>

#define SCVERSION "1.1"

void start_speedchecker_init(void)
{
	char uuid[37];
	char change = 0;
	if (!nvram_exists("speedchecker_uuid")) {
		if (getUUID(uuid)) {
			nvram_set("speedchecker_uuid", uuid);
			change = 1;
		}
	}

	if (!nvram_exists("speedchecker_uuid2")) {
		if (getUUID(uuid)) {
			nvram_set("speedchecker_uuid2", uuid);
			change = 1;
		}
	}
	if (change)
		nvram_async_commit();
}

void start_speedchecker(void)
{
	char wan_if_buffer[33];
	start_speedchecker_init();
	if (nvram_matchi("speedchecker_enable", 1)) {
		sysprintf(
			"SCC_JID=\"%s@xmpp.speedcheckerapi.com/%s|%s|ddwrt|%s|\" SCC_SRV=\"xmpp.speedcheckerapi.com\" SCC_STATS_IF=%s SCC_RNAME=\"%s\" scc &\n", //
			nvram_safe_get("speedchecker_uuid"), //
			SCVERSION, //
			PSVN_REVISION, //
			nvram_safe_get("os_version"), //
			safe_get_wan_face(wan_if_buffer), //
			nvram_safe_get("DD_BOARD"));
		dd_loginfo("speedchecker", "client started\n");
	}

	return;
}

void stop_speedchecker(void)
{
	stop_process("scc", "speedchecker");
	return;
}
#endif
