/*
 * lltd.c
 *
 * Copyright (C) 2011 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_LLTD
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_lltd(void);

void start_lltd(void)
{
	int ret;

	if (nvram_matchi("lltd_enabled", 0)) {
		stop_lltd();
		return;
	}
	/*
	 * Make sure its not running first 
	 */
	stop_process("lld2d", "daemon");
	log_eval("lld2d", nvram_safe_get("lan_ifname"));
	return;
}

void stop_lltd(void)
{
	stop_process("lld2d", "daemon");
	return;
}

#endif
