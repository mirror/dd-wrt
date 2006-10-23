/*
 * snmp.c
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

#ifdef HAVE_SNMP

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <snmp.h>

int
start_snmp (void)
{
  int ret = 0;
  pid_t pid;

  char fname[] = "/var/snmp/snmpd.conf";
  char *snmpd_argv[] = { "/usr/sbin/snmpd", "-c", fname, NULL };
  FILE *fp = NULL;

  stop_snmp ();

  if (!nvram_invmatch ("snmpd_enable", "0"))
    return 0;

  fp = fopen (fname, "w");
  if (NULL == fp)
    return -1;

  /* if smnpd_conf is defined and not the default use the contents as the config file */
  if (strlen (nvram_safe_get ("snmpd_conf")) > 0
      && strcmp (nvram_safe_get ("snmpd_conf"),
		 "See http://www.net-snmp.org for expert snmpd.conf options"))
    fprintf (fp, "%s", nvram_safe_get ("snmpd_conf"));
  else
    {
      if (strlen (nvram_safe_get ("snmpd_syslocation")) > 0)
	fprintf (fp, "syslocation %s\n",
		 nvram_safe_get ("snmpd_syslocation"));
      if (strlen (nvram_safe_get ("snmpd_syscontact")) > 0)
	fprintf (fp, "syscontact %s\n", nvram_safe_get ("snmpd_syscontact"));
      if (strlen (nvram_safe_get ("snmpd_sysname")) > 0)
	fprintf (fp, "sysname %s\n", nvram_safe_get ("snmpd_sysname"));
      if (strlen (nvram_safe_get ("snmpd_rocommunity")) > 0)
	fprintf (fp, "rocommunity %s\n",
		 nvram_safe_get ("snmpd_rocommunity"));
      if (strlen (nvram_safe_get ("snmpd_rwcommunity")) > 0)
	fprintf (fp, "rwcommunity %s\n",
		 nvram_safe_get ("snmpd_rwcommunity"));
    }
   fprintf (fp, "pass_persist .1.3.6.1.4.1.2021.255 /etc/wl_snmpd.sh\n");

  fclose (fp);
//  eval ("insmod", "wlcompat");
  ret = _eval (snmpd_argv, NULL, 0, &pid);

  cprintf ("done\n");

  return ret;
}

int
stop_snmp (void)
{
  int ret;

  ret=killall("snmpd",SIGKILL);
  cprintf ("done\n");

  return ret;
}
#endif
