/*
 * poe.c
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
#include <stdlib.h>
#include <ddnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void start_poe(void)
{
	if (nvram_match("poe", "1") && pidof("realtek-poe") == -1)
		log_eval("realtek-poe");
	else
		eval("ubus", "call", "poe", "reload");
	return;
}

void stop_poe(void)
{
	//stop_process("realtek-poe", "realtek poe daemon");
	return;
}

void restart_poe(void)
{
	eval("ubus", "call", "poe", "reload");
}
