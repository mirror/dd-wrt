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


  char *igmp_proxy_argv[] = { "igmprt",
    "-i",
    get_wan_face (),
    NULL
  };
  char *igmp_proxybr_argv[] = { "igmprt",
    "-i",
    nvram_safe_get("lan_ifname"),
    NULL
  };

  stop_igmp_proxy ();

  if (nvram_match ("block_multicast", "0"))
    {
      if (nvram_match("wan_proto","disabled"))
      ret = _eval (igmp_proxybr_argv, NULL, 0, &pid);
      else        
      ret = _eval (igmp_proxy_argv, NULL, 0, &pid);
      syslog (LOG_INFO, "igmprt : multicast daemon successfully started\n");
    }

  cprintf ("done\n");
  return ret;
}

int
stop_igmp_proxy (void)
{
  if (pidof ("igmprt") > 0)
    syslog (LOG_INFO, "igmprt : multicast daemon successfully stopped\n");
  int ret = killall ("igmprt", SIGTERM);

  cprintf ("done\n");
  return ret;
}
#endif
