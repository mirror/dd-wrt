/*
 * milkfish.c
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
#ifdef HAVE_MILKFISH
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void
stop_milkfish (void)
{
  if (pidof ("rtpproxy") > 0 || pidof ("openserctl") > 0)
    syslog (LOG_INFO, "Milkfish service successfully stopped\n");

  killall ("rtpproxy", SIGTERM);
  killall ("openserctl", SIGTERM);
}

void
start_milkfish (void)
{
  if (nvram_match ("milkfish_enabled", "1"))
    {
      eval ("/etc/config/milkfish.sh");
      eval ("/etc/config/milkfish.netup");	//start rtpproxy and openserctl

      syslog (LOG_INFO, "Milkfish service successfully started\n");
    }
}

#endif
