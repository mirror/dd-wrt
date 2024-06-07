/*
 * ipv6.c
 *
 * Copyright (C) 2022 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_IPV6
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> /* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <net/route.h> /* AhMan March 18 2005 */
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>
#include <syslog.h>
#include <services.h>

void stop_ipv6(void)
{
	rmmod("ipcomp6 xfrm6_tunnel xfrm6_mode_tunnel xfrm6_mode_transport xfrm6_mode xfrm6_mode_beet ip6_tunnel tunnel6 mip6 ah6 esp6 ipv6");
	if (nvram_match("wan_proto", "disabled")) {
		sysprintf("echo 1 > /proc/sys/net/ipv6/conf/%s/accept_ra", nvram_safe_get("lan_ifname"));
	} else {
		char wan_if_buffer[33];
		char *wan_ifname = safe_get_wan_face(wan_if_buffer);
		sysprintf("echo 1 > /proc/sys/net/ipv6/conf/%s/accept_ra", wan_ifname);
	}
	sysprintf("echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6");
	dd_loginfo("ipv6", "successfully stopped\n");
}

void start_ipv6(void)
{
	int ret = 0;

	if (!nvram_invmatchi("ipv6_enable", 0)) {
		sysprintf("echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6");
		return;
	}

	insmod("ipv6 tunnel4 ip_tunnel sit xfrm_algo esp6 ah6 mip6 tunnel6 ip6_tunnel xfrm6_mode_beet xfrm6_mode_ro xfrm6_mode_transport xfrm6_mode_tunnel xfrm6_tunnel xfrm_ipcomp ipcomp6");

	if (nvram_match("wan_proto", "disabled")) {
		sysprintf("echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", nvram_safe_get("lan_ifname"));
	} else {
		char wan_if_buffer[33];
		char *wan_ifname = safe_get_wan_face(wan_if_buffer);
		sysprintf("echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", wan_ifname);
	}
	sysprintf("echo 0 > /proc/sys/net/ipv6/conf/all/disable_ipv6");
	dd_loginfo("ipv6", "successfully started\n");

	cprintf("done\n");
	return;
}
#endif
