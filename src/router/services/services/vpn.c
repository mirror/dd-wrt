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
#if defined(HAVE_XSCALE) || defined(HAVE_FONERA) || defined(HAVE_RB600) || defined(HAVE_LAGUNA) || defined(HAVE_WHRAG108) || defined(HAVE_X86) ||defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5) || defined(HAVE_RT2880) || defined(HAVE_BCMMODERN) || defined(HAVE_LSX) || defined(HAVE_AP83) || defined(HAVE_AP96)

	if ((nvram_match("pptp_pass", "1") || nvram_match("l2tp_pass", "1")
	     || nvram_match("ipsec_pass", "1"))) {
		insmod("nf_conntrack_proto_gre");
		dd_syslog(LOG_INFO,
			  "vpn modules : nf_conntrack_proto_gre successfully loaded\n");
		insmod("nf_nat_proto_gre");
		dd_syslog(LOG_INFO,
			  "vpn modules : nf_nat_proto_gre successfully loaded\n");
	}
	if (nvram_match("pptp_pass", "1")) {
		insmod("nf_conntrack_pptp");
		dd_syslog(LOG_INFO,
			  "vpn modules : nf_conntrack_pptp successfully loaded\n");
		insmod("nf_nat_pptp");
		dd_syslog(LOG_INFO,
			  "vpn modules : nf_nat_pptp successfully loaded\n");
	}
#else
	if ((nvram_match("pptp_pass", "1") || nvram_match("l2tp_pass", "1")
	     || nvram_match("ipsec_pass", "1"))) {
		insmod("ip_conntrack_proto_gre");
		dd_syslog(LOG_INFO,
			  "vpn modules : ip_conntrack_proto_gre successfully loaded\n");
		insmod("ip_nat_proto_gre");
		dd_syslog(LOG_INFO,
			  "vpn modules : ip_nat_proto_gre successfully loaded\n");
	}
	if (nvram_match("pptp_pass", "1")) {
		insmod("ip_conntrack_pptp");
		dd_syslog(LOG_INFO,
			  "vpn modules : ip_conntrack_pptp successfully loaded\n");
		insmod("ip_nat_pptp");
		dd_syslog(LOG_INFO,
			  "vpn modules : ip_nat_pptp successfully loaded\n");
	}
#endif
}

void stop_vpn_modules(void)
{
#if defined(HAVE_XSCALE) || defined(HAVE_FONERA) || defined(HAVE_RB600) || defined(HAVE_LAGUNA) || defined(HAVE_WHRAG108) || defined(HAVE_X86) ||defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5) || defined(HAVE_RT2880) || defined(HAVE_BCMMODERN) || defined(HAVE_LSX) || defined(HAVE_AP83) || defined(HAVE_AP96)
	rmmod("nf_nat_pptp");
	rmmod("nf_conntrack_pptp");
	rmmod("nf_nat_proto_gre");
	rmmod("nf_conntrack_proto_gre");
	dd_syslog(LOG_INFO,
		  "vpn modules : vpn modules successfully unloaded\n");
#else
	rmmod("ip_nat_pptp");
	rmmod("ip_nat_proto_gre");
	rmmod("ip_conntrack_pptp");
	rmmod("ip_conntrack_proto_gre");
	dd_syslog(LOG_INFO,
		  "vpn modules : vpn modules successfully unloaded\n");

#endif
}
