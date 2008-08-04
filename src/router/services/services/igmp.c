/*
 * igmp.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_MULTICAST
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>


int
start_igmp_proxy (void)
{
  int ret = 0;
  pid_t pid;
  char name[80], *next, *svbuf;
  char *argv[] = { "igmprt", NULL };



  stop_igmp_proxy ();
  int ifcount = 0;


  FILE *fp = fopen ("/tmp/igmpproxy.conf", "wb");
  fprintf (fp, "quickleave\n");
  fprintf (fp, "phyint %s upstream  ratelimit 0  threshold 1\n",
	   nvram_safe_get ("wan_iface"));
  if (nvram_match ("block_multicast", "0"))
    {
      fprintf (fp, "phyint %s downstream  ratelimit 0  threshold 1\n",
	       nvram_safe_get ("lan_ifname"));
      ifcount++;
    }
  else
    {
      fprintf (fp, "phyint %s disabled\n", nvram_safe_get ("lan_ifname"));
      fprintf (fp, "phyint %s:0 disabled\n", nvram_safe_get ("lan_ifname"));
    }
  char ifnames[256];
  getIfLists (ifnames, 256);
  foreach (name, ifnames, next)
  {
    if (strcmp (get_wan_face (), name) && strcmp (nvram_safe_get("lan_ifname"), var))
      {
	if (nvram_nmatch ("0", "%s_bridged", name)
	    && nvram_nmatch ("1", "%s_multicast", name))
	  {
	    fprintf (fp, "phyint %s downstream  ratelimit 0  threshold 1\n",
		     name);
	    ifcount++;
	  }
	else
	  fprintf (fp, "phyint %s disabled\n", name);
      }
  }
  fprintf (fp, "phyint lo disabled\n");
  fclose (fp);
  if (nvram_match ("wan_proto", "disabled"))	//todo: add upstream config
    {
//        ret = _evalpid (igmp_proxybr_argv, NULL, 0, &pid);
      return ret;
    }
  else
    {
      if (ifcount)
	{
	  ret = _evalpid (argv, NULL, 0, &pid);
	  syslog (LOG_INFO,
		  "igmprt : multicast daemon successfully started\n");
	}
    }

  cprintf ("done\n");
  return ret;
}

int
stop_igmp_proxy (void)
{
  int ret = 0;
  if (pidof ("igmprt") > 0)
    {
      syslog (LOG_INFO, "igmprt : multicast daemon successfully stopped\n");
      ret = killall ("igmprt", SIGKILL);
    }
  cprintf ("done\n");
  return ret;
}
#endif
