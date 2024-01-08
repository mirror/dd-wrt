/*
 * conntrack.c
 *
 * Copyright (C) 2017 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void start_conntrack(void)
{
	char *CONNTRACK_MAX = nvram_safe_get("ip_conntrack_max");
	char *CONNTRACK_TCP_TIMEOUTS =
		nvram_safe_get("ip_conntrack_tcp_timeouts");
	char *CONNTRACK_UDP_TIMEOUTS =
		nvram_safe_get("ip_conntrack_udp_timeouts");
	//      char buckets[128];
	//      sprintf(buckets, "%d", atoi(CONNTRACK_MAX) / 4);
	writeprocsysnet("ipv4/ip_conntrack_max", CONNTRACK_MAX);
	writeprocsysnet("ipv4/netfilter/ip_conntrack_max", CONNTRACK_MAX);
	writeprocsysnet("nf_conntrack_max", CONNTRACK_MAX);
	//      writeprocsysnet("netfilter/nf_conntrack_buckets", buckets);

	writeprocsysnet("ipv4/netfilter/ip_conntrack_tcp_timeout_established",
			CONNTRACK_TCP_TIMEOUTS);
	writeprocsysnet("netfilter/nf_conntrack_tcp_timeout_established",
			CONNTRACK_TCP_TIMEOUTS);

	writeprocsysnet("ipv4/netfilter/ip_conntrack_udp_timeout",
			CONNTRACK_UDP_TIMEOUTS);
	writeprocsysnet("netfilter/nf_conntrack_udp_timeout",
			CONNTRACK_UDP_TIMEOUTS);

	return;
}

void stop_conntrack(void)
{
	return;
}
