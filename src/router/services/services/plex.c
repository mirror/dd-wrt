/*
 * telnet.c
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
#ifdef HAVE_PLEX
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

char *plex_deps(void)
{
	return "plex_enable plex_appdir";
}

char *plex_proc(void)
{
	return "Plex Media Server";
}

void stop_plex(void);
void start_plex(void)
{
	char *plex_argv[] = { "/usr/lib/plexmediaserver/lib/run.sh",
			      nvram_safe_get("plex_appdir"), NULL };
	stop_plex();

	if (!nvram_invmatchi("plex_enable", 0))
		return;

	dd_logstart("plex", _evalpid(plex_argv, NULL, 0, NULL));

	cprintf("done\n");
	return;
}

void stop_plex(void)
{
	stop_process("Plex Media Serv", "daemon");
	stop_process("Plex Tuner Serv", "daemon");
	nvram_delstates(plex_deps());
	cprintf("done\n");
	return;
}
#endif
