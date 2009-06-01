/*
 * mssid.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <bcmutils.h>
#include <shutils.h>
#include "libbridge.h"
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <linux/sockios.h>
#include <wlutils.h>
#include <services.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

void start_config_macs(char *wlifname)	// reconfigure macs which
						// should fix the corerev 5
						// and 7 problem
{
	int unit = get_wl_instance(wlifname);
	char *vifs = nvram_nget("wl%d_vifs", unit);
	char *mbss = nvram_nget("wl%d_mbss", unit);
	char *next;
	char var[80];

	if (!strcmp(mbss, "0")) {
		if (vifs != NULL)
			foreach(var, vifs, next) {
			eval("wl", "-i", var, "down");
			eval("wl", "-i", var, "cur_etheraddr",
			     nvram_nget("%s_hwaddr", var));
			eval("wl", "-i", var, "bssid",
			     nvram_nget("%s_hwaddr", var));
			eval("wl", "-i", var, "up");
			}
	}
}

void do_mssid(char *wlifname)
{
	// bridge the virtual interfaces too
	struct ifreq ifr;
	int s;
	char *next;
	char var[80];
	char *vifs = nvram_nget("wl%d_vifs", get_wl_instance(wlifname));

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;
	if (vifs != NULL)
		foreach(var, vifs, next) {
		ether_atoe(nvram_nget("%s_hwaddr", var),
			   ifr.ifr_hwaddr.sa_data);
		strncpy(ifr.ifr_name, var, IFNAMSIZ);
		ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
		if (!nvram_nmatch("0", "%s_bridged", var)) {
			// ifconfig (var, IFUP, NULL, NULL);
			eval("ifconfig", var, "down");
			ioctl(s, SIOCSIFHWADDR, &ifr);
			eval("ifconfig", var, "up");
			br_add_interface(getBridge(var), var);
		} else {
			eval("ifconfig", var, "down");
			ioctl(s, SIOCSIFHWADDR, &ifr);
			eval("ifconfig", var, "up");
			ifconfig(var, IFUP, nvram_nget("%s_ipaddr", var),
				 nvram_nget("%s_netmask", var));
		}
		}
	close(s);
}

#ifndef HAVE_MADWIFI

void set_vifsmac(char *base)	// corrects hwaddr and bssid assignment
{
	struct ifreq ifr;
	int s;
	char *next;
	char var[80];
	char mac[80];
	char *vifs = nvram_nget("%s_vifs", base);

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;
	if (vifs != NULL)
		foreach(var, vifs, next) {
		wl_getbssid(var, mac);
		eval("ifconfig", var, "down");
		ether_atoe(mac, ifr.ifr_hwaddr.sa_data);
		ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
		strncpy(ifr.ifr_name, var, IFNAMSIZ);
		ioctl(s, SIOCSIFHWADDR, &ifr);
		eval("ifconfig", var, "up");
		}
	close(s);
}

void start_vifsmac(void)
{
	int cnt = get_wl_instances();
    int c;
    char name[32];
    
    for(c = 0; c < cnt; c++) {
	sprintf(name, "wl%d", c);
    set_vifsmac(name);
	}
}
#endif
