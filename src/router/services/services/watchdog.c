/*
 * watchdog.c
 *
 * Copyright (C) 2021 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifndef HAVE_MICRO
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void start_watchdog(void)
{
	if (!nvram_matchi("disable_watchdog", 1) && pidof("watchdog") <= 0) {
		log_eval("watchdog"); // system watchdog
	}
	return;
}

void stop_watchdog(void) // shall never be called
{
	stop_process("watchdog", "watchdog timer");
	return;
}
#endif
