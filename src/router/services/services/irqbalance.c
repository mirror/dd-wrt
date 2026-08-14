/*
 * irqbalance.c
 *
 * Copyright (C) 2013 - 2026 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_IRQBALANCE
	#include <stdlib.h>
	#include <ddnvram.h>
	#include <shutils.h>
	#include <utils.h>
	#include <syslog.h>
	#include <signal.h>
	#include <sys/stat.h>
	#include <services.h>

void start_irqbalance(void)
{
	if (nvram_match("irqbalance_enabled", "1")) {
		if (getlogicalcores() > 1) {
			mkdir("/var/run/irqbalance", 0777);
			eval("irqbalance", "-t", "10");
		}
	}
	return;
}

void stop_irqbalance(void)
{
	stop_process("irqbalance", "daemon");
	return;
}
#endif
