/*
 * sfe.c
 *
 * Copyright (C) 2019 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_SFE
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <wlutils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void start_sfe(void)
{
	insmod("shortcut-fe");
	insmod("shortcut-fe-ipv6");
	insmod("fast-classifier");
	if (nvram_match("sfe", "1")) {
		sysprintf("echo 1 > /sys/fast_classifier/skip_to_bridge_ingress");
		sysprintf("echo 0 > /sys/fast_classifier/stop");
		dd_loginfo("sfe", "shortcut forwarding engine successfully started\n");
	} else if (nvram_match("sfe", "2")) {
		sysprintf("echo 1 > /sys/fast_classifier/stop");
		sysprintf("echo 1 > /sys/fast_classifier/defunct_all");
		writeproc("/proc/ctf", "1");
		dd_loginfo("ctf", "fast path forwarding successfully started\n");
	} else {
		sysprintf("echo 1 > /sys/fast_classifier/stop");
		sysprintf("echo 1 > /sys/fast_classifier/defunct_all");
		writeproc("/proc/ctf", "0");
	}

	return;
}

void stop_sfe(void)
{
	sysprintf("echo 1 > /sys/fast_classifier/stop");
	sysprintf("echo 1 > /sys/fast_classifier/defunct_all");
	writeproc("/proc/ctf", "0");
	dd_loginfo("sfe", "shortcut forwarding engine successfully stopped\n");
	return;
}
#endif
