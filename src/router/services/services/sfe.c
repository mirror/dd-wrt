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
	if (nvram_match("sfe", "1")) {
		eval("modprobe","ipv6");
		insmod("shortcut-fe");
		insmod("shortcut-fe-ipv6");
		insmod("fast-classifier");
		writeproc("/proc/ctf", "0");
		sysprintf("echo 1 > /sys/kernel/debug/ecm/front_end_ipv4_stop");
		sysprintf("echo 1 > /sys/kernel/debug/ecm/front_end_ipv6_stop");
	        sysprintf("echo 1 > /sys/kernel/debug/ecm/ecm_db/defunct_all");
		rmmod("ecm");
		sysprintf("echo 1 > /sys/fast_classifier/skip_to_bridge_ingress");
		sysprintf("echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_no_window_check");

		dd_loginfo("sfe", "shortcut forwarding engine successfully started\n");
	} else if (nvram_match("sfe", "2")) {
		rmmod("fast-classifier");
		rmmod("shortcut-fe-ipv6");
		rmmod("shortcut-fe");
		sysprintf("echo 1 > /sys/kernel/debug/ecm/front_end_ipv4_stop");
		sysprintf("echo 1 > /sys/kernel/debug/ecm/front_end_ipv6_stop");
	        sysprintf("echo 1 > /sys/kernel/debug/ecm/ecm_db/defunct_all");
		rmmod("ecm");
		writeproc("/proc/ctf", "1");
		sysprintf("echo 0 > /proc/sys/net/netfilter/nf_conntrack_tcp_no_window_check");
		dd_loginfo("ctf", "fast path forwarding successfully started\n");
	} else if (nvram_match("sfe", "3")) { // ecm nss
		rmmod("fast-classifier");
		rmmod("shortcut-fe-ipv6");
		rmmod("shortcut-fe");
		writeproc("/proc/ctf", "0");
		eval8"insmod","qca-nss-sfe");
		eval("insmod","ecm","front_end_selection=1");
		sysprintf("echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_no_window_check");
		dd_loginfo("ecm-nss", "ecm-nss forwarding successfully started\n");
	} else if (nvram_match("sfe", "4")) { // ecm sfe
		rmmod("fast-classifier");
		rmmod("shortcut-fe-ipv6");
		rmmod("shortcut-fe");
		writeproc("/proc/ctf", "0");
		eval8"insmod","qca-nss-sfe");
		eval("insmod","ecm","front_end_selection=2");
		sysprintf("echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_no_window_check");
		dd_loginfo("ecm-nss", "ecm-nss forwarding successfully started\n");
	} else if (nvram_match("sfe", "5")) { // ecm sfe & nss
		rmmod("fast-classifier");
		rmmod("shortcut-fe-ipv6");
		rmmod("shortcut-fe");
		writeproc("/proc/ctf", "0");
		eval8"insmod","qca-nss-sfe");
		eval("insmod","ecm","front_end_selection=4");
		sysprintf("echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_no_window_check");
		dd_loginfo("ecm-nss", "ecm-nss forwarding successfully started\n");
	} else {
		rmmod("fast-classifier");
		rmmod("shortcut-fe-ipv6");
		rmmod("shortcut-fe");

		sysprintf("echo 1 > /sys/kernel/debug/ecm/front_end_ipv4_stop");
		sysprintf("echo 1 > /sys/kernel/debug/ecm/front_end_ipv6_stop");
	        sysprintf("echo 1 > /sys/kernel/debug/ecm/ecm_db/defunct_all");
		rmmod("ecm");
		writeproc("/proc/ctf", "0");
		sysprintf("echo 0 > /proc/sys/net/netfilter/nf_conntrack_tcp_no_window_check");
	}

	return;
}

void stop_sfe(void)
{
	rmmod("fast-classifier");
	rmmod("shortcut-fe-ipv6");
	rmmod("shortcut-fe");
	writeproc("/proc/ctf", "0");
	dd_loginfo("sfe", "shortcut forwarding engine successfully stopped\n");
	return;
}
#endif
