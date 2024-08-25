/*
 * wol.c
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

#ifdef HAVE_WOL

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
#include "snmp.h"
#include <signal.h>
#include <utils.h>
#include <services.h>

#define WOL_INTERVAL 15

char *wol_deps(void)
{
	return "wol_enable";
}

char *wol_proc(void)
{
	return "wol";
}

void stop_wol(void)
{
	stop_process("wol", "Wake On LAN Daemon");
	nvram_delstates(wol_deps());
}

void start_wol(void)
{
	char *wol_argv[] = { "wol", NULL };

	stop_wol();

	if (nvram_matchi("wol_enable", 0))
		return;

	_log_evalpid(wol_argv, NULL, 0, NULL);
}

#endif /* HAVE_WOL */
