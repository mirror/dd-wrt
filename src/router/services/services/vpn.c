/*
 * vpn.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <sys/ioctl.h>		/* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
#include <net/route.h>		/* AhMan March 18 2005 */
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

void start_vpn_modules(void)
{
	if ((nvram_match("pptp_pass", "1") || nvram_match("l2tp_pass", "1")
	     || nvram_match("ipsec_pass", "1"))) {
		insmod("nf_conntrack_proto_gre");
		insmod("ip_conntrack_proto_gre");
		dd_syslog(LOG_INFO, "vpn modules : nf_conntrack_proto_gre successfully loaded\n");
		insmod("nf_nat_proto_gre");
		insmod("ip_nat_proto_gre");
		dd_syslog(LOG_INFO, "vpn modules : nf_nat_proto_gre successfully loaded\n");
	}
	if (nvram_match("pptp_pass", "1")) {
		insmod("nf_conntrack_pptp");
		insmod("ip_conntrack_pptp");
		dd_syslog(LOG_INFO, "vpn modules : nf_conntrack_pptp successfully loaded\n");
		insmod("nf_nat_pptp");
		insmod("ip_nat_pptp");
		dd_syslog(LOG_INFO, "vpn modules : nf_nat_pptp successfully loaded\n");
	}
}

void stop_vpn_modules(void)
{
	rmmod("nf_nat_pptp");
	rmmod("nf_conntrack_pptp");
	rmmod("nf_nat_proto_gre");
	rmmod("nf_conntrack_proto_gre");
	rmmod("ip_nat_pptp");
	rmmod("ip_nat_proto_gre");
	rmmod("ip_conntrack_pptp");
	rmmod("ip_conntrack_proto_gre");
	dd_syslog(LOG_INFO, "vpn modules : vpn modules successfully unloaded\n");
}
