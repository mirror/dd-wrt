/*
 * sputnik.c
 *
 * Copyright (C) 2007 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_SPUTNIK_APD
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

/*
 * Sputnik APD Service Handling 
 */
void start_sputnik(void)
{
	// Only start if enabled
	if (!nvram_invmatchi("apd_enable", 0))
		return;
	insmod("ipt_mark ipt_mac xt_mark xt_mac");

	log_eval("sputnik");
	return;
}

void stop_sputnik(void)
{
	stop_process("sputnik", "daemon");
	return;
}

/*
 * END Sputnik Service Handling 
 */

#endif
