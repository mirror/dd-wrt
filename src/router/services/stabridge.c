/*
 * stabridge.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>


static void filterarp(char *ifname)
{
      eval("ebtables","-t","broute","-A","BROUTING","-p","ARP","-i",ifname,"--arp-mac-dst","!",nvram_safe_get("lan_hwaddr"),"--arp-ip-dst","!",nvram_safe_get("lan_ipaddr"),"-j","DROP");

}
void start_stabridge(void)
{

#ifdef HAVE_MADWIFI
if (nvram_match("ath0_mode","wet"))
#else
if (nvram_match("wl0_mode","wet"))
#endif
    {
      eval ("insmod", "ebtables");
      eval ("insmod", "ebtable_nat");
      eval ("insmod", "ebtable_broute");
      eval ("insmod", "ebtable_filter");
      eval ("insmod", "ebt_snat");
      eval ("insmod", "ebt_arp");
      eval ("insmod", "ebt_arpreply");
#ifdef HAVE_MAGICBOX
      filterarp("eth0");
      filterarp("eth1");
      filterarp("eth2");
#elif HAVE_GATEWORX
      filterarp("ixp0");
      filterarp("ixp1");
#elif HAVE_RB500
      filterarp("eth0");
      filterarp("eth1");
      filterarp("eth2");
#else  //Broadcom

char firstlanif[16];

		sscanf (nvram_safe_get("lan_ifnames"), "%s ", firstlanif);

	  filterarp(firstlanif);
#endif
      eval("ebtables","-t","nat","-A","POSTROUTING","-o",nvram_safe_get("wl0_ifname"),"-j","snat","--to-src",nvram_safe_get("wl0_hwaddr"),"--snat-target","ACCEPT");

/*		"\t-s <size>\t- Use MAC DB size. Default is %d if no in configuration found\n"
		"\t-w <devname>\t- Use wireless device name. Default is ath0 if no in configuration found\n"
		"\t-b <devname>\t- Use bridge device name. Default is br0 if no in configuration found\n"
		"\t-e <devname(s)>\t- Use ethernet device(s) name(s) separated by space. Default is eth0 if no in configuration found\n",
*/
#ifdef HAVE_MAGICBOX
      filterarp("eth0");
      filterarp("eth1");
      filterarp("eth2");
      eval("stabridge","-d","-w","ath0","-b","br0","-e","eth0 eth1 eth2");
#elif HAVE_GATEWORX
      filterarp("ixp0");
      filterarp("ixp1");
      eval("stabridge","-d","-w","ath0","-b","br0","-e","ixp0 ixp1");
#elif HAVE_RB500
      filterarp("eth0");
      filterarp("eth1");
      filterarp("eth2");
      eval("stabridge","-d","-w","ath0","-b","br0","-e","eth0 eth1 eth2");
#else //Broadcom

      eval("stabridge","-d","-w",nvram_safe_get("wl0_ifname"),"-b","br0","-e",firstlanif);

#endif
	
    
    }
}


void stop_stabridge(void)
{
#ifdef HAVE_MADWIFI
if (nvram_match("ath0_mode","wet"))
#else
if (nvram_match("wl0_mode","wet"))
#endif
    {
      eval ("killall","stabridge");
      eval ("rmmod", "ebt_arpreply");
      eval ("rmmod", "ebt_arp");
      eval ("rmmod", "ebt_snat");
      eval ("rmmod", "ebtable_filter");
      eval ("rmmod", "ebtable_broute");
      eval ("rmmod", "ebtable_nat");    
      eval ("rmmod", "ebtables");
    }

}