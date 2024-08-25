/*
 * rstats.c
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
#ifdef HAVE_P910ND
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_printer(void)
{
	stop_process("p910nd", "printer daemon");
}

void start_printer(void)
{
	if (nvram_matchi("usb_enable", 1) && nvram_matchi("usb_printer", 1)) {
		FILE *test = fopen("/dev/usb/lp0", "rb");
		if (!test) {
			eval("mknod", "/dev/lp0", "c", "180", "0");
			eval("mknod", "/dev/lp1", "c", "180", "1");
			eval("mknod", "/dev/lp2", "c", "180", "2");
			log_eval("p910nd", "-f", "/dev/lp0", "0");
		} else {
			fclose(test);
			log_eval("p910nd", "-f", "/dev/usb/lp0", "0");
		}
	}
}

#endif
