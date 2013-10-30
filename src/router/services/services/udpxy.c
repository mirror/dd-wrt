/*      
 * * udpxy.c
 * *      
 * * Copyright (C) 2013 Markus Quint <markus@dd-wrt.com>
 * *      
 * * This program is free software; you can redistribute it and/or
 * * modify it under the terms of the GNU General Public License
 * * as published by the Free Software Foundation; either version 2
 * * of the License.
 * *      
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * * GNU General Public License for more details.  
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  
 * *
 * * $Id:
 * */

#ifdef HAVE_UDPXY
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_udpxy(void);
void start_udpxy(void)
{
	char *nicfrom, *listen_if, *listen_port;
	char *cmd = "/usr/sbin/udpxy";

	if (!nvram_match("udpxy_enable", "1"))
		return;
	
	nicfrom = nvram_safe_get("tvnicfrom");
	listen_if = nvram_safe_get("udpxy_listenif");
	listen_port = nvram_safe_get("udpxy_listenport");

	if (!nicfrom || !listen_if || !listen_port)
		return;

	eval(cmd, "-m", nicfrom, "-a", listen_if, "-p", listen_port);
	
	dd_syslog(LOG_INFO, "udpxy : UDP-to-HTTP multicast traffic relay daemon started\n");

	return;	
}

void stop_udpxy(void)
{
	stop_process("udpxy", "UDP-to-HTTP multicast traffic relay daemon");
	dd_syslog(LOG_INFO, "udpxy : UDP-to-HTTP multicast traffic relay daemon stopped\n");
	return;
}
#endif
