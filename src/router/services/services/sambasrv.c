/*
 * sambasrv.c
 *
 * Copyright (C) 2008 dd-wrt
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
#ifdef HAVE_SAMBA_SRV
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>

void start_sambasrv(void)
{
	if (!nvram_match("sambasrv_enable", "1"))
		return;

	FILE *fp;

	// here comes the startup code

	eval("samba");
	syslog(LOG_INFO, "Samba : Samba server successfully started\n");

	return;
}

void stop_sambasrv(void)
{

	if (pidof("samba") > 0) {
		syslog(LOG_INFO, "Samba : samba server successfully stopped\n");
		killall("proftpd", SIGTERM);
	}
}
#endif
