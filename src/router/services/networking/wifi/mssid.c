/*
 * mssid.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <libbridge.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <linux/sockios.h>
#include <wlutils.h>
#include <services.h>
#include <utils.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

void config_macs(char *wlifname) // reconfigure macs which
	// should fix the corerev 5
	// and 7 problem
{
	int unit = get_wl_instance(wlifname);
	char *vifs = nvram_nget("wl%d_vifs", unit);
	char *mbss = nvram_nget("wl%d_mbss", unit);
	char *next;
	char var[80];

	if (!strcmp(mbss, "0") || nvram_nmatch("apsta", "wl%d_mode", unit) ||
	    nvram_nmatch("ap", "wl%d_mode", unit)) {
		foreach(var, vifs, next)
		{
			eval("ifconfig", var, "down");
			eval("wl", "-i", var, "down");
			eval("wl", "-i", var, "cur_etheraddr",
			     nvram_nget("%s_hwaddr", var));
			fprintf(stderr, "Setting %s BSSID:  %s \n", var,
				nvram_nget("%s_hwaddr", var));
			eval("wl", "-i", var, "bssid",
			     nvram_nget("%s_hwaddr", var));
			eval("wl", "-i", var, "up");
			eval("ifconfig", var, "up");
		}
	}
}
