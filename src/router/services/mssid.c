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


#ifdef HAVE_MSSID
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <libbridge.h>
#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

void
do_mssid (char *lan_ifname)
{
  //bridge the virtual interfaces too
  char *next;
  char var[80];
  char *vifs = nvram_safe_get ("wl0_vifs");
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      char bridged[32];
      sprintf (bridged, "%s_bridged", var);
      if (nvram_match (bridged, "1"))
	{
	  ifconfig (var, IFUP, NULL, NULL);
	  br_add_interface (lan_ifname, var);
	}
      else
	{
	  char ip[32];
	  char mask[32];
	  sprintf (ip, "%s_ipaddr", var);
	  sprintf (mask, "%s_netmask", var);
	  ifconfig (var, IFUP, nvram_safe_get (ip), nvram_safe_get (mask));
	}
      //  eval ("brctl", "addif", lan_ifname, var);
    }
}
#endif
