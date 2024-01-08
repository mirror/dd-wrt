/*
 * vpn.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

char *vpn_modules_deps(void)
{
	return "pptp_pass";
}

void stop_vpn_modules(void);

void start_vpn_modules(void)
{
	stop_vpn_modules();
	if (nvram_matchi("pptp_pass", 1)) {
		insmod("nf_conntrack_proto_gre ip_conntrack_proto_gre");
		dd_loginfo("vpn modules",
			   "nf_conntrack_proto_gre successfully loaded\n");
		insmod("nf_nat_proto_gre ip_nat_proto_gre");
		dd_loginfo("vpn modules",
			   "nf_nat_proto_gre successfully loaded\n");
		insmod("nf_conntrack_pptp ip_conntrack_pptp");
		dd_loginfo("vpn modules",
			   "nf_conntrack_pptp successfully loaded\n");
		insmod("nf_nat_pptp ip_nat_pptp");
		dd_loginfo("vpn modules", "nf_nat_pptp successfully loaded\n");
	}
}

void stop_vpn_modules(void)
{
	rmmod("nf_nat_pptp nf_conntrack_pptp nf_nat_proto_gre nf_conntrack_proto_gre ip_nat_pptp ip_nat_proto_gre ip_conntrack_pptp ip_conntrack_proto_gre");
	dd_loginfo("vpn modules", "vpn modules successfully unloaded\n");
	nvram_delstates(vpn_modules_deps());
}
